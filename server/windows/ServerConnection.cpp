#ifdef WINDOWS
#include "tcp/windows/ServerConnection.h"
#include "tcp/windows/Socket.h"

namespace c11http {
namespace tcp {
namespace windows {

ServerConnection::ServerConnection(const Socket& socket, Server* _server) :
        Overlap<ServerConnection>(), mSocket(socket.getSocket()), mBytes(0), mServer(_server) //, operation( Socket::OP_WAITING )
{
    SecureZeroMemory(mBuffer, sizeof(mBuffer) / sizeof(mBuffer[0]));

    mDataBuffer.len = sizeof(mBuffer);
    mDataBuffer.buf = mBuffer;
    createOverlap(this);
}

ServerConnection::~ServerConnection()
{
    closesocket(mSocket);
    mSocket = INVALID_SOCKET;
}

void ServerConnection::setIdentifier(const std::string& identifier)
{
    mIdentifier = identifier;
}

const SOCKET ServerConnection::getSocket() const
{
    return mSocket;
}
const DWORD& ServerConnection::getBytes() const
{
    return mBytes;
}
const char* ServerConnection::getBuffer() const
{
    return mBuffer;
}
LPWSABUF ServerConnection::getDataBuffer()
{
    return &mDataBuffer;
}
const DWORD& ServerConnection::getOperation() const
{
    return mOperation;
}
const std::string& ServerConnection::getIdentifier() const
{
    return mIdentifier;
}
Server* ServerConnection::getServer() const
{
    return mServer;
}

}
}
}

#endif
