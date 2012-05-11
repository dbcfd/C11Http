#ifdef WINDOWS
#include "tcp/windows/Socket.h"

#include <sstream>

namespace c11http {
namespace tcp {
namespace windows {

Socket::Socket(const SOCKET _sckt) :
        mSocket(_sckt)
{

}

SOCKET Socket::getSocket() const
{
    return mSocket;
}

Socket::Socket() throw(std::runtime_error)
{
    mSocket = INVALID_SOCKET;
    mSocket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_IP, NULL, 0, WSA_FLAG_OVERLAPPED);

    if(mSocket == INVALID_SOCKET)
    {
        std::stringstream sstr;
        sstr << "Socket Creation Error " << WSAGetLastError();
        throw(std::runtime_error(sstr.str()));
    }

    //
    // Disable send buffering on the socket.  Setting SO_SNDBUF
    // to 0 causes winsock to stop buffering sends and perform
    // sends directly from our buffers, thereby save one memory copy.
    //
    // However, this does prevent the socket from ever filling the
    // send pipeline. This can lead to packets being sent that are
    // not full (i.e. the overhead of the IP and TCP headers is
    // great compared to the amount of data being carried).
    //
    // Disabling the send buffer has less serious repercussions
    // than disabling the receive buffer.
    //
    int nZero = 0;
    int nRet = setsockopt(mSocket, SOL_SOCKET, SO_SNDBUF, (char*) &nZero, sizeof(nZero));

    if(nRet == SOCKET_ERROR)
    {
        std::stringstream sstr;
        sstr << "setsockopt(SNDBUF) failed:" << WSAGetLastError();
        //TODO: log
    }
}
Socket::~Socket()
{

}
void Socket::closeSocket()
{
    closesocket(mSocket);
}

}
}
}

#endif
