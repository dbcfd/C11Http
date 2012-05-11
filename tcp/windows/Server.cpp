#ifdef WINDOWS
#include "tcp/windows/Server.h"

#include <sstream>

#include "tcp/windows/ServerConnection.h"
#include "tcp/windows/Callback.h"

namespace c11http {
namespace tcp {
namespace windows {

/**
 * Callback invoked after server receives data from a client. Passes the client information to the
 * server callback
 */
void CALLBACK ClientDataReady(
        IN DWORD dwError,
        IN DWORD cbTransferred,
        IN LPWSAOVERLAPPED lpOverlapped,
        IN DWORD dwFlags
)
{
    ServerConnection::OverlapType* overlap = static_cast<ServerConnection::OverlapType*>( lpOverlapped );
    ServerConnection* client = overlap->derived;
    Server* server = client->getServer();

    server->queueReceiveFromConnection( client );

    Callback* callback = server->getCallback();

    if( nullptr != callback ) callback->receiveComplete(client->getBuffer(), client->getBytes());
}

/**
 * Callback invoked after data has been sent to client.
 */
void CALLBACK DataSentToClient(
        IN DWORD dwError,
        IN DWORD cbTransferred,
        IN LPWSAOVERLAPPED lpOverlapped,
        IN DWORD dwFlags
)
{
    ServerConnection::OverlapType* overlap = static_cast<ServerConnection::OverlapType*>( lpOverlapped );
    ServerConnection* client = overlap->derived;
    Server* server = client->getServer();

    if( server->isShutdown() ) return;

    ServerOverlapData* serverData = static_cast<ServerOverlapData*>( server->getOverlap()->derived );
    std::string target = serverData->targets.front();
    serverData->targets.pop();

    Callback* callback = server->getCallback();

    WSAResetEvent( overlap->hEvent );

    if( 0 != callback ) callback->sendComplete( target, cbTransferred );
}

Server::Server(Callback* _callback, const unsigned int port) throw(std::runtime_error) :
        mCallback(_callback), mServerSocket(NULL), mIncomingSocket(NULL), mHasBeenShutdown(false), mWaitingForEvents(
                false)
{
    //startup winsock
    WSAData wsaData;
    int iResult;
    iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);

    if(iResult != 0)
    {
        std::stringstream sstr;
        sstr << "WSAStartup Error " << iResult;
        throw(std::runtime_error(sstr.str()));
    }

    mOverlapData.server = this;

    createOverlap(&mOverlapData);

    createServerSocket(port);

    prepareForServerConnection();
}

Server::~Server()
{
    shutdown();
    WSARecvDisconnect(mServerSocket->getSocket(), 0);
    WSASendDisconnect(mServerSocket->getSocket(), 0);
    mServerSocket->closeSocket();
    delete mServerSocket;
    mServerSocket = 0;
    WSARecvDisconnect(mIncomingSocket->getSocket(), 0);
    WSASendDisconnect(mIncomingSocket->getSocket(), 0);
    mIncomingSocket->closeSocket();
    delete mIncomingSocket;
    mIncomingSocket = 0;
}

void Server::addConnection(ServerConnection* client, const std::string& identifier)
{
    mConnections.addServerConnection(client, identifier);
}

const bool Server::isShutdown() const
{
    return mHasBeenShutdown;
}

Callback* Server::getCallback()
{
    return mCallback;
}

void Server::shutdown()
{
    mHasBeenShutdown = true;
    std::vector<ServerConnection*> connectionPtrs = mConnections.getConnections();

    for(std::vector<ServerConnection*>::const_iterator iter = connectionPtrs.begin();
            iter != connectionPtrs.end(); ++iter)
    {
        ServerConnection* connection = (*iter);

        WSARecvDisconnect(connection->getSocket(), 0);
        WSASendDisconnect(connection->getSocket(), 0);

        mConnections.removeServerConnection(connection->getOverlap()->hEvent);
    }

    WSASetEvent(getOverlap()->hEvent);}

void Server::send(const char* data, const unsigned int count, const std::string& identifier)
        throw(std::runtime_error)
{
    ServerConnection* connection = mConnections.getServerConnection(identifier);

    send(data, count, connection, true);

    mOverlapData.targets.push(identifier);

}

void Server::send(const char* data, const unsigned int count, ServerConnection* connection,
        const bool reportToCallback) throw(std::runtime_error)
{
    WSABUF dataBuffer;
    DWORD flags = 0;
    DWORD nbBytes = 0;
    dataBuffer.len = count;
    dataBuffer.buf = (CHAR*) data;

    mOverlapData.targets.push(connection->getIdentifier());

    /**
     * Queue an asynchronous send to a specific client. This should complete
     * immediately, invoking the callback when send i/o completes.
     */
    if(SOCKET_ERROR
            == WSASend(connection->getSocket(), &dataBuffer, 1, &nbBytes, flags,
                    connection->getOverlap(), DataSentToClient))
    {
        if(WSA_IO_PENDING != WSAGetLastError())
        {
            std::stringstream sstr;
            sstr << "error sending data to socket " << connection->getSocket() << " - "
                    << WSAGetLastError();

            if(reportToCallback)
            {
                Callback* callback = getCallback();

                if(0 != callback)
                    callback->sendFailed(connection->getIdentifier(), sstr.str());

                mConnections.removeServerConnection(connection->getIdentifier());
            }
        }
    }
    //small send, completed immediately, report to callback
    else if(reportToCallback)
    {
        Callback* callback = getCallback();

        if(0 != callback)
            callback->sendComplete(connection->getIdentifier(), count);
    }
}

void Server::broadcast(const char* data, const unsigned int count)
{
    std::vector<ServerConnection*> connectionPtrs = mConnections.getConnections();

    for(std::vector<ServerConnection*>::const_iterator iter = connectionPtrs.begin();
            iter != connectionPtrs.end(); ++iter)
    {
        ServerConnection* connection = (*iter);

        send(data, count, connection, true);
    }
}

HANDLE Server::performWaitForEvents() throw(std::runtime_error)
{

    DWORD wait_rc = WSA_WAIT_IO_COMPLETION;

    std::vector < WSAEVENT > handles;

    /**
     * Wait until i/o is completed on the handle triggered. A handle
     * can be triggered without i/o being complete.
     */
    while(WSA_WAIT_IO_COMPLETION == wait_rc && !mHasBeenShutdown) //can receive shutdown while still waiting on io completion
    {
        handles = mConnections.getHandleArray(getOverlap()->hEvent);

        if(WSA_WAIT_FAILED
                == (wait_rc = WSAWaitForMultipleEvents(handles.size(), &(handles[0]), FALSE,
                        WSA_INFINITE, TRUE)))
        {
            int error = WSAGetLastError();

            std::stringstream sstr;
            sstr << "WSAWaitForMultipleEvents Error " << WSAGetLastError();
            throw(std::runtime_error(sstr.str()));
        }
    }

    if(mHasBeenShutdown)
        return handles[0]; //return a handle within range if shut down

    HANDLE handle = handles[(wait_rc - WSA_WAIT_EVENT_0)];
    return handle;
}

void Server::waitForEvents() throw(std::runtime_error)
{

    while(!mHasBeenShutdown)
    {
        mWaitingForEvents = true;
        HANDLE handleId = performWaitForEvents();

        if(!mHasBeenShutdown) //if not shutdown, handle returned is valid and should be examined
        {
            WSAResetEvent(handleId);

            if(getOverlap()->hEvent == handleId)
            {
                //if the event triggered is the server event, add a new connection
                addNewConnection();
            }
            else
            {
                /**
                 * Handle a receive event for one of the server connections, send
                 * events are handled via a callback.
                 */
                ServerConnection* connection = mConnections.getServerConnection(handleId);
                DWORD flags = 0;
                DWORD nbBytes = 0;

                //make sure connection exists and recv has completed
                if(connection
                        && WSAGetOverlappedResult(connection->getSocket(),
                                (WSAOVERLAPPED*) connection->getOverlap(), &nbBytes, TRUE, &flags))
                {
                    //received data from the connection
                    Callback* callback = getCallback();

                    if(0 != callback)
                    {
                        if(0 == nbBytes)
                        {
                            callback->disconnected(connection->getIdentifier());
                            mConnections.removeServerConnection(handleId);
                        }
                        else
                        {
                            callback->receiveComplete(connection->getIdentifier(),
                                    connection->getBuffer(), nbBytes);
                        }
                    }

                    //queue the socket to receive again
                    if(!mHasBeenShutdown && 0 != nbBytes)
                    {
                        queueReceiveFromConnection(connection);
                    }
                }
            }
        }
    }

    mWaitingForEvents = false;
}

const bool Server::isWaitingForEvents() const
{
    return mWaitingForEvents;
}

void Server::addNewConnection() throw(std::runtime_error)
{
    BOOL bOptVal = TRUE;
    int bOptLen = sizeof(BOOL);
    //update the socket context based on our server
    setsockopt(mIncomingSocket->getSocket(), SOL_SOCKET, SO_UPDATE_ACCEPT_CONTEXT, (char*) &bOptVal,
            bOptLen);
    ServerConnection* connection = new ServerConnection(*mIncomingSocket, this);
    delete mIncomingSocket;
    mIncomingSocket = NULL;

    DWORD flags = 0;
    DWORD nbBytes = 0;

    /**
     * When a connection is formed, we will be receiving information from that client at
     * some point in time. We queue a receive that will call back to a method that
     * can handle the connection information.
     */
    int iResult = WSARecv(connection->getSocket(), //socket
            connection->getDataBuffer(), //WSABuf array
            1, //number of buffers
            &nbBytes, //bytes received
            &flags, //flags
            (WSAOVERLAPPED*) connection->getOverlap(), //overlap
            ConnectionInformationReceivedFromClient); //callback

    if(SOCKET_ERROR == iResult)
    {
        int lastError = WSAGetLastError();

        if(WSA_IO_PENDING != lastError)
        {
            std::stringstream sstr;
            sstr << "Error prepping client socket for receive " << WSAGetLastError();
            throw(std::runtime_error(sstr.str()));
        }
    }

    //prepare for another client to connect
    prepareForServerConnection();
}

void Server::queueReceiveFromConnection(ServerConnection* connection) throw(std::runtime_error)
{
    DWORD flags = 0;

    WSAResetEvent(connection->getOverlap()->hEvent);

    int iResult = WSARecv(connection->getSocket(), connection->getDataBuffer(), 1, 0, &flags,
            (WSAOVERLAPPED*) connection->getOverlap(), 0);

    if(SOCKET_ERROR == iResult)
    {
        int lastError = WSAGetLastError();

        if(WSA_IO_PENDING != lastError)
        {
            std::stringstream sstr;
            sstr << "Error prepping client socket for receive" << WSAGetLastError();
            throw(std::runtime_error(sstr.str()));
        }
    }
}

void Server::createServerSocket(const unsigned int port) throw(std::runtime_error)
{
    //WSA startup successful, create address info
    struct addrinfo* result = NULL, *ptr = NULL, hints;

    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_PASSIVE;

    std::stringstream sstr;
    sstr << port;

    int iResult = getaddrinfo("127.0.0.1", sstr.str().c_str(), &hints, &result);

    if(iResult != 0)
    {
        std::stringstream sstr;
        sstr << "getaddrinfo Error " << iResult;
        freeaddrinfo(result);
        WSACleanup();
        throw(std::runtime_error(sstr.str()));
    }

    //getaddrinfo successful, create socket
    mServerSocket = new Socket();

    //socket creation successful, bind to socket
    iResult = bind(mServerSocket->getSocket(), result->ai_addr, (int) result->ai_addrlen);

    if(SOCKET_ERROR == iResult)
    {
        std::stringstream sstr;
        sstr << "bind Error " << WSAGetLastError();
        freeaddrinfo(result);
        throw(std::runtime_error(sstr.str()));
    }

    //socket ready to go
    freeaddrinfo(result);

    if(SOCKET_ERROR == listen(mServerSocket->getSocket(), 5)) //TODO: backlog config
    {
        std::stringstream sstr;
        sstr << "listen Error " << WSAGetLastError();
        mServerSocket->closeSocket();
        WSACleanup();
        throw(std::runtime_error(sstr.str()));
    }
}

void Server::prepareForServerConnection() throw(std::runtime_error)
{
    LPFN_ACCEPTEX pfnAcceptEx;
    GUID acceptex_guid = WSAID_ACCEPTEX;
    DWORD bytes = 0, connectionBytesRead = 0;

    //create a socket to accept the connection on
    mIncomingSocket = new Socket();

    //use i/o control to set up the socket for accept ex
    if(SOCKET_ERROR
            == WSAIoctl(mServerSocket->getSocket(), SIO_GET_EXTENSION_FUNCTION_POINTER,
                    &acceptex_guid, sizeof(acceptex_guid), &pfnAcceptEx, sizeof(pfnAcceptEx),
                    &bytes, NULL, NULL))
    {
        std::stringstream sstr;
        sstr << "Failed to obtain AcceptEx() pointer";
        throw(std::runtime_error(sstr.str()));
    }

    /**
     * Setup accept ex to accept a connection to the server asynchronously when
     * it occurs. We read nothing from the socket, since we want to accept immediately,
     * then acknowledge to the client that a connection has occurred.
     */
    if(!pfnAcceptEx(mServerSocket->getSocket(), mIncomingSocket->getSocket(), mAcceptExBuffer,
            0, //read nothing from the socket
            sizeof(SOCKADDR_STORAGE) + 16, sizeof(SOCKADDR_STORAGE) + 16, &connectionBytesRead,
            getOverlap()))
    {

        int lasterror = WSAGetLastError();

        if(WSA_IO_PENDING != lasterror)
        {
            std::stringstream sstr;
            sstr << "AcceptEx Error()";
            throw(std::runtime_error(sstr.str()));
        }
    }
}

}
}
}

#endif
