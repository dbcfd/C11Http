#pragma once

#include <vector>

#include "tcp/posix/Platform.h"
#include "tcp/posix/posix.h"

namespace c11http {
namespace tcp {
namespace posix {

class Callback;

/**
 * Posix implementation of a client, which is a connection to some server. This class is implemented asynchronously
 * and will not create any additional threads.
 */
class TCP_POSIX_API Client
{
public:
    /**
     * Connection a specific server listening at hostname:port. Users are notified of events via the specified
     * callback, and the identifier is sent to the server on connection.
     */
    Client(Callback* callback, const std::string& hostname,
            const unsigned int port, const std::string& identifier)
                    throw (std::runtime_error);
    ~Client();

    /**
     * Block the current thread, waiting for events. This will unblock when data is ready to be sent/recv'd or
     * a disconnection request is received.
     */
    virtual void waitForEvents() throw (std::runtime_error);
    /**
     * Send data to the connected server.
     */
    virtual void sendDataToServer(const char* data, const unsigned int count)
            throw (std::runtime_error);
    /**
     * Disconnect from the server, closing the connection.
     */
    virtual void disconnect() throw (std::runtime_error);

    const bool isConnected() const;

    const int getSocket() const;
	const char* getBuffer() const;
private:
    Callback* getCallback() const;

    /**
     * Perform the wait action.
     */
    void performWait() throw (std::runtime_error);
    /**
     * Create a wakeup pipe, that uses a self-pipe to unblock this object.
     */
    void prepareWakeupPipe() throw (std::runtime_error);
    /**
     * Receive data from the server, into a buffer. Socket must be ready for reading, as indicated by a select/poll
     */
    int receiveDataFromServer();
    /**
     * Unblock this client, using a self-pipe.
     */
    void performWakeup();

    Callback* mCallback;
    int mWakeupPipe[2];
    int mFdMax;
    bool mConnected;
    std::string mIdentifier;
    fd_set mMasterWrite;
    int mSocket;
    char mBuffer[MAX_BUFFER_SIZE];

    std::vector<char> mOutgoingBytes;
};

}
}
}
