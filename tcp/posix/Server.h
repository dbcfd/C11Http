#pragma once

#include <vector>

#include "tcp/posix/Platform.h"
#include "tcp/posix/posix.h"

namespace c11http {
namespace tcp {
namespace posix {

class Callback;
class Socket;
class Connections;
class ServerConnection;

/**
 * Implementation of a server using posix calls. Will listen for connections on a port, accept those connections
 * and send/recv data between the connections. Is implemented asynchronously using non-blocking sockets and self
 * piping. This class does not create any other threads.
 */
class TCP_POSIX_API Server
{
public:
    /**
     * Create a server listening on the specified port, notifying users of events with the specified callback.
     */
    Server(Callback* callback, const unsigned int port)
            throw (std::runtime_error);
    ~Server();

    /**
     * Send a message to all connections.
     */
    void broadcast(const char* data, const unsigned int count)
            throw (std::runtime_error);
    /**
     * Send a message to a specific connection, as indicated by the identifier.
     */
    void send(const char* data, const unsigned int count,
            const std::string& identifier) throw (std::runtime_error);
    /**
     * Blocks the current thread, waiting until an event occurs. Events include connection attempts, sending/receiving
     * data, and shutdown.
     */
    void waitForEvents() throw (std::runtime_error);
    /**
     * Shutdown this server, closing all connections.
     */
    void shutdown();

    Callback* getCallback() const;

private:
    /**
     * Utilize self-pipe to unblock.
     */
    void performWakeup();
    /**
     * Create a ServerConnection based on a connection attempt.
     */
    void handleServerConnection(int sckt);
    /**
     * Send data to a specified ServerConnection
     */
    void send(const char* data, const unsigned int count,
            ServerConnection* connection) throw (std::runtime_error);

    Socket* mConnectSocket;
    Connections* mConnections;
    char mBuffer[MAX_BUFFER_SIZE];
    const unsigned int mPort;
    Callback* mCallback;
    bool mHasBeenShutdown;
    int mWakeupPipe[2];
    fd_set mMasterWrite;
};

}
}
}
