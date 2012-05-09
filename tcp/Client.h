#pragma once

#include "tcp/Platform.h"

namespace c11http {
namespace tcp {

/**
 * Generic implementation of a client. Receives data from a platform implementation and then passes that 
 * information along to a worker.
 */
class TCP_API Client
{
public:
	/**
	 * Connection a specific server listening at hostname:port. Users are notified of events via the specified
	 * callback, and the identifier is sent to the server on connection.
	 */
	Client(const std::string& hostname, const unsigned int port, const std::string& onConnectMessage);
	~Client();

	/**
	 * Block the current thread, waiting for events. This will unblock when data is ready to be sent/recv'd or
	 * a disconnection request is received.
	 */
	void waitForEvents();
	/**
	 * Send data to the connected server.
	 */
	void sendDataToServer(const char* data, const unsigned int count);
	/**
	 * Disconnect from the server, closing the connection.
	 */
	void disconnect();

private:
	void receiveComplete(const std::string& identifier, const char* data, const unsigned int count);

	class PlatformCallback;
	class PlatformClient;

	PlatformCallback* mCallback;
	PlatformClient* mClient;
};

}
}

