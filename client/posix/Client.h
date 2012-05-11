#pragma once

#include <vector>

#include "client/interface/IClient.h"

#include "client/posix/Platform.h"
#include "client/posix/posix.h"

namespace c11http {
namespace client {
namespace posix {

class Callback;

/**
 * Posix implementation of a client, which is a connection to some server. This class is implemented asynchronously
 * and will not create any additional threads.
 */
class CLIENT_POSIX_API Client : public IClient
{
public:
    /**
     * Connection a specific server listening at hostname:port. Users are notified of events via the specified
     * callback, and the identifier is sent to the server on connection.
     */
    Client(Callback* callback, const std::string& hostname, const unsigned int port, const std::string& identifier);;
    ~Client();

    /**
     * Block the current thread, waiting for events. This will unblock when data is ready to be sent/recv'd or
     * a disconnection request is received.
     */
    virtual void connectToServer(const std::string& hostname, const unsigned int port);
    /**
     * Send data to the connected server.
     */
    virtual objects::HttpResponse sendRequestToServer(const objects::HttpRequest& req);
    /**
     * Disconnect from the server, closing the connection.
     */
    virtual void disconnect();
private:
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
