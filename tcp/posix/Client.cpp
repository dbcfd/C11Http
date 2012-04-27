#ifndef WINDOWS
#include "tcp/posix/Client.h"

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

Client::Client(Callback* _callback, const std::string& hostname,
		const unsigned int port, const std::string& _identifier)
				throw (std::runtime_error) :
		mCallback(_callback), mConnected(false), mIdentifier(_identifier) {

	Socket connectSocket;
	mSocket = connectSocket.getSocket();

	/**
	 * Convert our address information into a usable format
	 */
	struct sockaddr_in server;
	int result = inet_pton(AF_INET, hostname.c_str(), &server.sin_addr.s_addr);

	/**
	 * Check for errors and report them
	 */
	if (0 >= result) {
		if (0 == result) {
			std::stringstream sstr;
			sstr << "not in presentation format";
			throw(std::runtime_error(sstr.str()));
		} else {
			std::stringstream sstr;
			sstr << "inet_pton Error " << strerror(errno);
			throw(std::runtime_error(sstr.str()));
		}
	}

	/**
	 * Set port and type
	 */
	memset(&server, 0, sizeof(server));
	server.sin_family = AF_INET;
	server.sin_port = htons(port);

	/**
	 * Connect to our server, on the previously defined socket (file descriptor)
	 */
	result = connect(mSocket, (struct sockaddr*) &server, sizeof(server));

	if (-1 == result) {
		//throw error
		std::stringstream sstr;
		sstr << "failed to connect to host server " << strerror(errno);
		throw(std::runtime_error(sstr.str()));
	}

	//receive acknowledge from server
	char ackBuff[4];
	::recv(mSocket, ackBuff, sizeof(ackBuff), 0);
	//send over our connection information
	::send(mSocket, _identifier.c_str(), _identifier.size(), 0);

	mConnected = true;

	/**
	 * Now that we're connected, we can do asynchronous i/o, so need a non-blocking socket
	 */
	connectSocket.makeNonBlocking();

	prepareWakeupPipe();
}

Client::~Client() {
	if (mConnected)
		disconnect();
}

Callback* Client::getCallback() const {
	return mCallback;
}

const bool Client::isConnected() const {
	return mConnected;
}

const int Client::getSocket() const {
	return mSocket;
}
const char* Client::getBuffer() const {
	return mBuffer;
}

void Client::prepareWakeupPipe() throw (std::runtime_error) {
	if (-1 == pipe(mWakeupPipe)) {
		std::stringstream sstr;
		sstr << "Failed to create wakeup pipe " << strerror(errno);
		throw(std::runtime_error(sstr.str()));
	}

	mFdMax = std::max(mWakeupPipe[0], mSocket);
}

void Client::waitForEvents() throw (std::runtime_error) {
	/**
	 * Prepare our file descriptor sets for select
	 */
	fd_set masterRead, readFds, writeFds;
	FD_ZERO(&masterRead);
	FD_ZERO(&mMasterWrite);
	FD_SET(mWakeupPipe[0], &masterRead);
	FD_SET(mSocket, &masterRead);

	while (mConnected) {
		//copy from the master into a temporary
		readFds = masterRead;
		writeFds = mMasterWrite;

		/**
		 * Wait until a file descriptor is ready, this blocks
		 */
		if (-1 == select(mFdMax + 1, &readFds, &writeFds, 0, 0)) {
			std::stringstream sstr;
			sstr << "Select error " << strerror(errno);
			throw(std::runtime_error(sstr.str()));
		}

		if (!mConnected)
			break;

		/**
		 * Check all client connection sockets to see if there is information to read
		 */
		if (FD_ISSET(mSocket, &readFds)) //read from socket
				{
			/**
			 * Socket is ready, block until all data is read
			 */
			int nbytes = ::recv(mSocket, &mBuffer, sizeof(mBuffer),
					MSG_WAITALL);

			if (-1 == nbytes) {
				std::stringstream sstr;
				sstr << "Failed to recv from socket: " << strerror(errno);
				throw(std::runtime_error(sstr.str()));
			}

			if (0 == nbytes) {
				//server closed connection
				mConnected = false;
			} else {
				Callback* receiveCB = getCallback();

				if (0 != receiveCB)
					receiveCB->receiveComplete(mIdentifier, mBuffer, nbytes);
			}
		}

		/**
		 * See if we were attempting to wake up the client, for send/disconnect
		 */
		if (FD_ISSET(mWakeupPipe[0], &readFds)) //performing wakeup
				{
			//clear the wakeup pipe
			::recv(mWakeupPipe[0], &mBuffer, sizeof(mBuffer), MSG_WAITALL);
		}

		/**
		 * Data is ready to be sent, and the client sockets are ready to receive data
		 */
		if (FD_ISSET(mSocket, &writeFds)) //send data to server
				{
			std::vector<char> bytesToSend;
			bytesToSend.swap(mOutgoingBytes);

			const int expected = bytesToSend.size();
			bool sendSuccessful = true;

			if (expected
					!= ::send(mSocket, &(bytesToSend.front()), expected, 0)) {
				sendSuccessful = false;
			}

			Callback* sendCB = getCallback();

			if (0 != sendCB)
				sendCB->sendComplete(mIdentifier, expected);

			/**
			 * Write is complete, clear the socket from the write list
			 */
			FD_CLR(mSocket, &mMasterWrite);
		}
	}

}

void Client::sendDataToServer(const char* data, const unsigned int count)
		throw (std::runtime_error) {
	if (count > 0) {
		mOutgoingBytes.reserve(mOutgoingBytes.size() + count);
		mOutgoingBytes.insert(mOutgoingBytes.begin(), data, data + count);
		/**
		 * Need to add our socket to the write list now that it has data ready. If
		 * we added it earlier with no data available, it would constantly be shown
		 * as ready by the select call. It is ready since it has no data, and can
		 * write immediately.
		 */
		FD_SET(mSocket, &mMasterWrite);
		/**
		 * Use our wakeup pipe since the current set of write file descriptors (write_fds)
		 * does not contain the socket we want to write to. Self pipe wakes us up and
		 * allows us to add it to the set.
		 */
		performWakeup();
	}
}

/**
 * Self pipe technique for wake up from select
 */
void Client::performWakeup() {
	char wakeup = '.';
	struct pollfd fd;
	fd.fd = mWakeupPipe[1];
	fd.events = POLLOUT;
	::poll(&fd, 1, -1);
	::write(mWakeupPipe[1], &wakeup, 1);
}

void Client::disconnect() throw (std::runtime_error) {
	if (mConnected) {
		mConnected = false;
		struct pollfd fd;
		fd.fd = mSocket;
		fd.events = POLLOUT;

		if (-1 == poll(&fd, 1, -1)) {
			std::stringstream sstr;
			sstr << "Error polling for socket write " << strerror(errno);
			throw(std::runtime_error(sstr.str()));
		}

		::send(mSocket, mBuffer, 0, 0);
		Socket sckt(mSocket);
		sckt.closeSocket();
		performWakeup();
	}
}

}
}
}

#endif
