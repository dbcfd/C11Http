#ifndef WINDOWS
#include "tcp/posix/Server.h"

#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <string.h>
#include <sstream>
#include <algorithm>
#include <aio.h>
#include <poll.h>

#include "tcp/posix/ServerConnection.h"
#include "tcp/posix/Connections.h"
#include "tcp/posix/Socket.h"
#include "tcp/posix/Callback.h"

namespace c11http {
namespace tcp {
namespace posix {

Server::Server(Callback* _callback, const unsigned int _port)
        throw (std::runtime_error)
        : mPort(_port), mCallback(_callback), mHasBeenShutdown(false)
{
    int result = 0;
    //create a socket to accept connections/data on
    mConnectSocket = new Socket();

    if (-1
            == setsockopt(mConnectSocket->getSocket(), SOL_SOCKET, SO_REUSEADDR,
                    &result, sizeof(int)))
    {
        std::stringstream sstr;
        sstr << "Error reusing socket file descriptor " << strerror(errno);
        throw(std::runtime_error(sstr.str()));
    }

    /**
     * Define our server connection information
     */
    struct sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY; //use any available address, gives us localhost
    server.sin_port = htons(mPort);
    memset(&(server.sin_zero), '\0', 8);

    //bind our connection socket
    if (-1
            == bind(mConnectSocket->getSocket(), (struct sockaddr*) &server,
                    sizeof(server)))
    {
        std::stringstream sstr;
        sstr << "bind Error " << strerror(errno);
        throw(std::runtime_error(sstr.str()));
    }

    //have our connection socket start listening
    if (-1 == listen(mConnectSocket->getSocket(), 10))
    {
        std::stringstream sstr;
        sstr << "listen error " << strerror(errno);
        throw(std::runtime_error(sstr.str()));
    }

    //everything setup, time to make asynchronous
    mConnectSocket->makeNonBlocking();

    //wakeup pipe to cancel out select when send/disconnect ready
    if (-1 == pipe(mWakeupPipe))
    {
        std::stringstream sstr;
        sstr << "Failed to create wakeup pipe " << strerror(errno);
        throw(std::runtime_error(sstr.str()));
    }
}

void Server::waitForEvents() throw (std::runtime_error)
{
    //setup file descriptors
    fd_set masterRead;
    fd_set readFds;
    fd_set writeFds;
    FD_ZERO(&masterRead);
    FD_SET(mWakeupPipe[0], &masterRead);
    FD_SET(mConnectSocket->getSocket(), &masterRead);
    mConnections = new Connections(masterRead);
    int serverMax = std::max(mWakeupPipe[0], mConnectSocket->getSocket());

    while (!mHasBeenShutdown)
    {
        int numfds = 0;
        int fdmax = std::max(serverMax, mConnections->getMax());

        FD_ZERO(&readFds);
        FD_ZERO(&writeFds);

        //copy our master list of file descriptors
        readFds = masterRead;
        writeFds = mMasterWrite;

        /**
         * Select from specified file descriptors, will block
         */
        numfds = select(fdmax + 1, &readFds, &writeFds, NULL, 0);

        if (-1 == numfds)
        {
            std::stringstream sstr;
            sstr << "select error " << strerror(errno);
            throw(std::runtime_error(sstr.str()));
        }

        if (mHasBeenShutdown)
            break;

        /**
         * Check all the file descriptors to see which were hit
         */
        for (int i = 0; i <= fdmax; ++i)
        {
            if (FD_ISSET(i, &readFds)) //handle reads
            {
                /**
                 * If connectSocket is ready, that means a connection is formed, any other
                 * socket (per client connection) shows data is ready to be ready from a
                 * client
                 */
                if (i == mConnectSocket->getSocket())
                {
                    //handle new connection
                    try
                    {
                        ServerConnection* client = new ServerConnection(
                        		mConnectSocket->getSocket()); //performs accept, gets identifier
                        getCallback()->connected(client->getIdentifier());
                        mConnections->addServerConnection(client);
                    } catch (std::runtime_error& ex)
                    {
                        //TODO: log connection failure
                        //std::cout << "failure: " << ex.what() << std::endl;
                    }
                }
                else
                {
                    //creating a new connection to a client
                    handleServerConnection(i);
                }
            }

            /**
             * Self-pipe technique to wakeup and add additional file descriptors
             * to select set.
             */
            if (FD_ISSET(mWakeupPipe[0], &readFds)) //performing wakeup
            {
                char buffer[1];
                ::recv(mWakeupPipe[0], &mBuffer, sizeof(mBuffer), 0);
            }

            /**
             * See if we're ready to write to a client connection
             */
            if (FD_ISSET(i, &writeFds))
            {
                //ready for write
                ServerConnection* connection = mConnections->getServerConnection(
                        i);
                if (0 != connection)
                {
                    connection->sendQueuedMessage(getCallback());
                }
                //write complete, clear it from the select list
                FD_CLR(i, &mMasterWrite);
            }
        }
    }

}

/**
 * Self pipe technique for wake up from select
 */
void Server::performWakeup()
{
    char wakeup = '.';
    struct pollfd fd;
    fd.fd = mWakeupPipe[1];
    fd.events = POLLOUT;
    ::poll(&fd, 1, -1);
    ::write(mWakeupPipe[1], &wakeup, 1);
}

/**
 * Socket associated with client connection has indicated it has data ready
 * to read. We want to determine which client connection and receive the data
 * from it.
 */
void Server::handleServerConnection(int sckt)
{
    //determine connection
    ServerConnection* connection = mConnections->getServerConnection(sckt);
    if (0 != connection)
    {
        Callback* callback = getCallback();
        try
        {
            //receive data
            std::vector<char> result = connection->performReceive();

            if (0 != callback)
            {
                callback->receiveComplete(connection->getIdentifier(), &(result[0]),
                        result.size());
            }

        } catch (std::runtime_error)
        {
            if (0 != callback)
            {
                callback->disconnected(connection->getIdentifier());
            }
            //error retrieving data from client, connection is bad, need to remove it
            mConnections->removeServerConnection(sckt);
        }
    }
}

Server::~Server()
{
    if (!mHasBeenShutdown)
        shutdown();
    mConnectSocket->closeSocket();
    delete mConnectSocket;
    mConnectSocket = 0;
    delete mConnections;
    mConnections = 0;
}

void Server::broadcast(const char* byteStream, const unsigned int count)
        throw (std::runtime_error)
{
    std::vector<ServerConnection*>& conns = mConnections->getConnections();
    for (std::vector<ServerConnection*>::iterator iter = conns.begin();
            iter != conns.end(); ++iter)
    {
        send(byteStream, count, (*iter));
    }
    performWakeup();
}

void Server::send(const char* data, const unsigned int count,
        ServerConnection* connection) throw (std::runtime_error)
{
    /**
     * Need to add our socket to the write list now that it has data ready. If
     * we added it earlier with no data available, it would constantly be shown
     * as ready by the select call. It is ready since it has no data, and can
     * write immediately.
     */
    FD_SET(connection->getSocket(), &mMasterWrite);
    connection->addQueuedMessage(data, count);
}

void Server::send(const char* data, const unsigned int count,
        const std::string& identifier) throw (std::runtime_error)
{
    ServerConnection* connection = mConnections->getServerConnection(identifier);
    if (0 != connection)
    {
        send(data, count, connection);

    }
    else
    {
        std::stringstream sstr;
        sstr << "Connection not found: " << identifier;
        throw(std::runtime_error(sstr.str()));
    }
    performWakeup();

}

void Server::shutdown()
{
    mHasBeenShutdown = true;
    mConnectSocket->closeSocket();
    performWakeup();
}

Callback* Server::getCallback() const
{
    return mCallback;
}

}
}
}

#endif
