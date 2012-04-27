#pragma once

#include <queue>

#include "tcp/windows/Overlap.h"

namespace c11http {
namespace tcp {
namespace windows {

class Callback;

/**
 * Winsock2 implementation of a client, which is a connection to some server. This class is implemented asynchronously
 * and will not create any additional threads.
 */
class TCP_WINDOWS_API Client: public Overlap<Client>
{
public:
	/**
	 * Connection a specific server listening at hostname:port. Users are notified of events via the specified
	 * callback, and the identifier is sent to the server on connection.
	 */
	Client(Callback* callback, const std::string& hostname,
			const unsigned int port, const std::string& onConnectMessage)
	throw (std::runtime_error);
	~Client();

	/**
	 * Block the current thread, waiting for events. This will unblock when data is ready to be sent/recv'd or
	 * a disconnection request is received.
	 */
	void waitForEvents() throw (std::runtime_error);
	/**
	 * Send data to the connected server.
	 */
	void sendDataToServer(const char* data, const unsigned int count)
	throw (std::runtime_error);
	/**
	 * Disconnect from the server, closing the connection.
	 */
	void disconnect() throw (std::runtime_error);
	/**
	 * Prepare asynchronous calls such that client can receive data. After each receive of data, a new asynchronous
	 * callback must be prepared.
	 */
	void prepareClientToReceiveData() throw (std::runtime_error);

	Callback* getCallback();
	const bool isConnected() const;

	const SOCKET getSocket() const;
	const DWORD& getBytes() const;
	const char* getBuffer() const;
	const WSABUF& getDataBuffer() const;
	const DWORD& getOperation() const;
	const std::string& getIdentifier() const;

private:
	SOCKET mSocket;
	DWORD mBytes;
	char mBuffer[MAX_BUFFER_SIZE];
	WSABUF mDataBuffer;
	DWORD mOperation;
	std::string mIdentifier;
	Overlap<Client>* mSendOverlap;
	Callback* mCallback;
	bool mConnected;
	std::queue<char*> mSendBuffers;
};

}
}
}

