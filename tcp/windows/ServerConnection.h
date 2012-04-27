#pragma once

#include "tcp/windows/Overlap.h"
#include "tcp/windows/Winsock2.h"

namespace c11http {
namespace tcp {
namespace windows {

class Socket;
class Server;

/**
 * Represent a connection between a server and a client. An underlying socket
 * is used for communication between the server and client.
 */
class TCP_WINDOWS_API ServerConnection: public Overlap<ServerConnection>
{
public:
	/**
	 * Extension of Overlapped structure used by asynchronous calls. See Overlap for more information
	 */
	struct ServerConnectionOverlap: public WSAOVERLAPPED
	{
		ServerConnection* connection;
	};

	/**
	 * Utilizes the socket created through an asynchronous accept (AcceptEx), to maintain a connection
	 * between a server and a client
	 */
	ServerConnection(const Socket& socket, Server* server)
	throw (std::runtime_error);
	~ServerConnection();

	void setIdentifier(const std::string& identifier);

	const SOCKET getSocket() const;
	const DWORD& getBytes() const;
	const char* getBuffer() const;
	LPWSABUF getDataBuffer();
	const DWORD& getOperation() const;
	const std::string& getIdentifier() const;
	Server* getServer() const;

private:
	SOCKET mSocket; //socket
	DWORD mBytes;//bytes received or sent
	char mBuffer[MAX_BUFFER_SIZE];//storage buffer
	WSABUF mDataBuffer;//Winsock2 specific buffer for send/recv
	Server* mServer;//Server that has this connection
	std::string mIdentifier;
	DWORD mOperation;

};

}
}
}
