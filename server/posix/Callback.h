#pragma once

#include <string>

#include "tcp/posix/Platform.h"

namespace c11http {
namespace tcp {
namespace posix {

/**
 * Callback used to identify users that actions have occurred on a connection.
 */
class TCP_POSIX_API Callback
{
public:
    Callback()
    {
    }
    virtual ~Callback()
    {
    }

    /**
     * A send to a connection has completed, with count bytes sent.
     */
    virtual void sendComplete(const std::string& identifier,
            const unsigned int count) = 0;
    /**
     * A send to a connection has failed, due to message.
     */
    virtual void sendFailed(const std::string& identifier,
            const std::string& message) = 0;
    /**
     * A receive from a connection has completed, with data of size count received.
     */
    virtual void receiveComplete(const std::string& identifier,
            const char* data, const unsigned int count) = 0;
    /**
     * A connection has been established.
     */
    virtual void connected(const std::string& connectedTo)= 0;
    /**
     * A connection has been terminated.
     */
    virtual void disconnected(const std::string& connectedTo) = 0;

};

}
}
}
