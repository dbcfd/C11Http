#pragma once

#include "tcp/posix/Platform.h"
#include "tcp/posix/posix.h"

namespace c11http {
namespace tcp {
namespace posix {

/**
 * Wrapper around the underlying posix file descriptor, which maintains
 * a network connection to some other device.
 */
class TCP_POSIX_API Socket
{
public:
    /**
     * Create a socket, using next available file descriptor, setting appropriate attribution.
     */
    Socket() throw (std::runtime_error);
    /**
     * Provide a wrapper around an existing socket. This allows us to use wrapper functionality
     * on a socket that may have been created in another method (e.g. accept)
     */
    Socket(int _sckt);
    ~Socket();

    /**
     * Close the underlying file descriptor
     */
    void closeSocket();
    /**
     * Makes this socket non-blocking. Sends and receives will complete immediately if possible,
     * returning information about their capabilities in regards to send and receive.
     */
    void makeNonBlocking();
    /**
     * Makes this socket blocking. If the file descriptor is not ready, current thread is blocked.
     */
    void makeBlocking();

    const int getSocket() const;
private:

    int mSocket;
};

}
}
}
