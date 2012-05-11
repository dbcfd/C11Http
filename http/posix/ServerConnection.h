#pragma once

#include <vector>

#include "tcp/posix/Platform.h"
#include "tcp/posix/posix.h"

namespace c11http {
namespace tcp {
namespace posix {

class Callback;

/**
 * Represent a connection between a server and a client. An underlying file descriptor (socket)
 * is used for communication between the server and client.
 */
class TCP_POSIX_API ServerConnection
{
public:
    /**
     * Accept an incoming connection on the server listening socket, creating a new connection between
     * the server and the client for sending and receiving data.
     */
    ServerConnection(const int serverSocket) throw (std::runtime_error);
    ~ServerConnection();

    /**
     * Add a message to send to the client. When the socket is available for writing, the message will be sent.
     */
    void addQueuedMessage(const char* data, const unsigned int count);
    /**
     * Send a queued message to the client. Socket must be available for writing, and will not block when
     * when the write occurs, as indicated by a select or poll.
     */
    void sendQueuedMessage(Callback* callback);
    /**
     * Receive data from a client, storing it in a vector. Data must be available on the socket, as indicated
     * by a select or poll operation.
     */
    std::vector<char> performReceive() throw (std::runtime_error);

    const int getSocket() const;
	const char* getBuffer() const;
	const std::string& getIdentifier() const;

private:
    std::vector<char> mOutgoingBytes;
    int mSocket; //file descriptor of socket
    char mBuffer[MAX_BUFFER_SIZE]; //buffer to store send/recv information in
    std::string mIdentifier; //identifier of this server connection
};

}
}
}
