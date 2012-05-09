#pragma once

#include "tcp/Platform.h"

namespace c11http {
namespace tcp {

/**
 * Generic implementation of a server. Receives data from a platform implementation (posix/winsock) and then
 * passes that information along to a worker.
 */
class TCP_API Server {
public:
    /**
     * Create a server listening on the specified port, notifying users of events with the specified callback.
     */
    Server(const unsigned int port);
    ~Server();

    /**
     * Blocks the current thread, waiting until an event occurs. Events include connection attempts, sending/receiving
     * data, and shutdown.
     */
    void waitForEvents();
    /**
     * Send a message to a specific connection, as indicated by the identifier.
     */
    void send(const char* data, const unsigned int count, const std::string& identifier);
    /**
     * Send a message to all connections.
     */
    void broadcast(const char* data, const unsigned int count);
    /**
     * Shutdown this server, closing all connections.
     */
    void shutdown();

private:
	void receiveComplete(const std::string& identifier, const char* data, const unsigned int count);

	class PlatformCallback;
	class PlatformServer;

	PlatformCallback* mCallback;
	PlatformServer* mServer;
};

}
}
