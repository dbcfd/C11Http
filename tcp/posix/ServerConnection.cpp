#ifndef WINDOWS
#include "tcp/posix/ServerConnection.h"

#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <sstream>
#include <poll.h>

#include "tcp/posix/Socket.h"
#include "tcp/posix/Callback.h"

namespace c11http {
namespace tcp {
namespace posix {

ServerConnection::ServerConnection(const int serverSocket)
		throw (std::runtime_error) {
	struct sockaddr_in server;
	socklen_t serversize = sizeof(server);
	/**
	 * Accept a new connection on the server socket. A new socket will
	 * be created that can be used to send/recv data to a client.
	 */
	int possibleSocket = accept(serverSocket, (struct sockaddr*) &server,
			&serversize);

	if (-1 == possibleSocket) {
		std::stringstream sstr;
		sstr << "failed to handle new connection: " << strerror(errno);
		throw(std::runtime_error(sstr.str()));
	}

	//make our socket non-blocking for asynch i/o
	Socket acceptSocket(possibleSocket);
	mSocket = acceptSocket.getSocket();
	acceptSocket.makeNonBlocking();

	//send an acknowledgement to the client, acknowledging connection
	std::string ack("ack");
	::send(mSocket, ack.c_str(), ack.size(), 0);

	//wait for data back from the client, indicating who the client is
	pollfd fd;
	fd.fd = mSocket;
	fd.events = POLLIN | POLLPRI;

	//poll blocks current thread until receive occurs
	if (-1 == poll(&fd, 1, -1)) {
		std::stringstream sstr;
		sstr << "failed to poll socket for receive: " << strerror(errno);
		throw(std::runtime_error(sstr.str()));
	}

	//receive data from the client, creating the identifier
	std::vector<char> res = performReceive();
	mIdentifier = std::string(&res[0], res.size());
}

std::vector<char> ServerConnection::performReceive() throw (std::runtime_error) {
	std::vector<char> result;
	bool shouldRead = true;

	/**
	 * At this point, we have polled/selected and know the socket is ready to recv
	 * from. This function is only called when a poll/select has been performed.
	 * Since the socket has data available, we want to read all available data,
	 * and this recv will block until all data has been sent by the client.
	 */
	int nbytes = recv(mSocket, mBuffer, MAX_BUFFER_SIZE, MSG_WAITALL);
	if (-1 == nbytes) {
		std::stringstream sstr;
		sstr << "failed to recv from socket: " << strerror(errno);
		throw(std::runtime_error(sstr.str()));
	}
	/**
	 * A zero size send indicates the client is attempting to close the connection
	 */
	if (0 == nbytes) {
		std::stringstream sstr;
		sstr << "remote device closing connection";
		throw(std::runtime_error(sstr.str()));
	}
	result.reserve(result.size() + nbytes);
	result.insert(result.end(), mBuffer, mBuffer + nbytes);
	return result;
}

ServerConnection::~ServerConnection() {
    Socket sckt(mSocket);
    sckt.closeSocket();
}

const int ServerConnection::getSocket() const {
	return mSocket;
}
const char* ServerConnection::getBuffer() const {
	return mBuffer;
}
const std::string& ServerConnection::getIdentifier() const {
	return mIdentifier;
}

void ServerConnection::addQueuedMessage(const char* data,
		const unsigned int count) {
	mOutgoingBytes.reserve(mOutgoingBytes.size() + count);
	mOutgoingBytes.insert(mOutgoingBytes.begin(), data, data + count);
}

void ServerConnection::sendQueuedMessage(Callback* callback) {
	std::vector<char> bytesToSend;
	//use the closest a vector has to an atomic operation to get data to send
	mOutgoingBytes.swap(bytesToSend);

	const int expected = bytesToSend.size();

	/**
	 * At this point, a poll/select has been performed that indicates
	 * that this socket is available for sending. We can then send
	 * all of our available data.
	 */
	if (expected == ::send(mSocket, &(bytesToSend[0]), expected, 0)) {
		callback->sendComplete(mIdentifier, expected);
	}
}

}
}
}

#endif
