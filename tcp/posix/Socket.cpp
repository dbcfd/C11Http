#ifndef WINDOWS

#include "tcp/posix/Socket.h"

#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <fcntl.h>
#include <sstream>
#include <string.h>

namespace c11http {
namespace tcp {
namespace posix {

Socket::Socket() throw (std::runtime_error) {
	mSocket = -1;
	mSocket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);

	if (-1 == mSocket) {
		std::stringstream sstr;
		sstr << "Socket Creation Error  " << strerror(errno);
		throw(std::runtime_error(sstr.str()));
	}
}

Socket::Socket(int _sckt) {
	mSocket = _sckt;
}

const int Socket::getSocket() const {
	return mSocket;
}

void Socket::makeNonBlocking() {
	int x;
	x = fcntl(mSocket, F_GETFL, 0);
	fcntl(mSocket, F_SETFL, x | O_NONBLOCK);
}

void Socket::makeBlocking() {
	int x;
	x = fcntl(mSocket, F_GETFL, 0);
	fcntl(mSocket, F_SETFL, x & ~O_NONBLOCK);
}

Socket::~Socket() {

}

void Socket::closeSocket() {
	::shutdown(mSocket, SHUT_RDWR);
	struct linger noLinger;
	noLinger.l_onoff = 0;
	noLinger.l_linger = 0;
	setsockopt(mSocket, SOL_SOCKET, SO_LINGER,
	                    &noLinger, sizeof(linger));
	::close(mSocket);
}

}
}
}

#endif