#pragma once

#include <queue>

#include "tcp/windows/Overlap.h"
#include "tcp/windows/Connections.h"
#include "tcp/windows/Socket.h"

namespace c11http {
namespace tcp {
namespace windows {

class Callback;
class Server;

/**
 * Extension of asynchronous overlap data used to identify a server.
 */
struct ServerOverlapData
{
    Server* server;
    std::queue<std::string> targets;
};

/**
 * Implementation of a server using Winsock2 calls. Will listen for connections on a port, accept those connections
 * and send/recv data between the connections. Is implemented asynchronously using non-blocking sockets.
 * This class does not create any other threads.
 */
class TCP_WINDOWS_API Server: public Overlap<ServerOverlapData>
{
public:
    /**
     * Create a server listening on the specified port, notifying users of events with the specified callback.
     */
    Server(Callback* callback, const unsigned int port)
            throw (std::runtime_error);
    ~Server();

    /**
     * Blocks the current thread, waiting until an event occurs. Events include connection attempts, sending/receiving
     * data, and shutdown.
     */
    void waitForEvents() throw (std::runtime_error);
    /**
     * Send a message to a specific connection, as indicated by the identifier.
     */
    void send(const char* data, const unsigned int count,
            const std::string& identifier) throw (std::runtime_error);
    /**
     * Send a message to all connections.
     */
    void broadcast(const char* data, const unsigned int count)
            throw (std::runtime_error);
    /**
     * Shutdown this server, closing all connections.
     */
    void shutdown();
    const bool isWaitingForEvents() const;

    /**
     * Queue an asynchronous call to receive data from a connection.
     */
    void queueReceiveFromConnection(ServerConnection* client)
            throw (std::runtime_error);
    /**
     * Add a newly formed connection to this server, once an identifier has been received.
     */
    void addConnection(ServerConnection* client, const std::string& identifier);
    Callback* getCallback();
    const bool isShutdown() const;

private:
    /**
     * Receive data from a connection that has triggered a receive event on the handle
     */
    void handleDataFromConnection(HANDLE handleId) throw (std::runtime_error);
    /**
     * The listening socket has a connection ready to be added.
     */
    void addNewConnection() throw (std::runtime_error);
    /**
     * Wait until a handle has some event occurring (e.g. connection, send, recv)
     */
    HANDLE performWaitForEvents() throw (std::runtime_error);
    /**
     * Create the socket that we listen on for new connections
     */
    void createServerSocket(const unsigned int port) throw (std::runtime_error);
    /**
     * Create the asynchronous socket and functionality necessary for AcceptEx
     */
    void prepareForServerConnection() throw (std::runtime_error);
    /**
     * Send data a connection, reporting to a callback if necessary
     */
    void send(const char* data, const unsigned int count,
            ServerConnection* connection, const bool reportToCallback)
                    throw (std::runtime_error);

    Socket* mServerSocket; //this is the socket we listen for connections on
    Socket* mIncomingSocket; //this is the socket that will be used when a connection occurs, as specified in AcceptEx
    Connections mConnections;
    ServerOverlapData mOverlapData;
    Callback* mCallback;

    //buffer used when connections accepted
    char mAcceptExBuffer[2 * (sizeof(SOCKADDR_STORAGE) + 16)];
    bool mHasBeenShutdown;
    bool mWaitingForEvents;

};

}
}
}
