#pragma once

#include "tcp/windows/Platform.h"

namespace c11http {
namespace tcp {
namespace windows {

/**
 * Callback used to identify users that actions have occurred on a connection.
 */
class TCP_WINDOWS_API Callback
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
    virtual void sendComplete(const unsigned int count) = 0;
    /**
     * A send to a connection has failed, due to message.
     */
    virtual void sendFailed(const std::string& message) = 0;
    /**
     * A receive from a connection has completed, with data of size count received.
     */
    virtual void receiveComplete(const char* data, const unsigned int count) = 0;
    /**
     * A connection has been established.
     */
    virtual void connected()= 0;
    /**
     * A connection has been terminated.
     */
    virtual void disconnected() = 0;

};

}
}
}
