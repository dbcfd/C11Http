
#ifdef WINDOWS
#include "tcp/windows/Client.h"

#include <sstream>

#include "tcp/windows/Socket.h"
#include "tcp/windows/Callback.h"

namespace c11http {
namespace tcp {
namespace windows {

Client::Client(Callback* _callback, const std::string& hostname, const unsigned int port,
        const std::string& onConnectMessage) throw(std::runtime_error) :
        mCallback(_callback), mBytes(0), mOperation(Socket::OP_WAITING), mIdentifier(hostname), mConnected(
                false)
{
    //startup winsock
    WSAData wsaData;
    int iResult;
    iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);

    if(0 != iResult)
    {
        std::stringstream sstr;
        sstr << "WSAStartup Error " << iResult;
        WSACleanup();
        throw(std::runtime_error(sstr.str()));
    }

    //define where we're connecting to
    struct addrinfo* results = NULL, *addrptr, hints;
    ZeroMemory(&hints, sizeof(hints));

    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_PASSIVE;

    std::stringstream sstr;
    sstr << port;

    //get the address information for the host
    iResult = getaddrinfo(hostname.c_str(), sstr.str().c_str(), &hints, &results);

    if(0 != iResult)
    {
        std::stringstream sstr;
        sstr << "getaddrinfo failed: " << iResult;
        WSACleanup();
        throw(std::runtime_error(sstr.str()));
    }

    if(NULL == results)
    {
        std::stringstream sstr;
        sstr << "no results for server " << hostname;
        WSACleanup();
        throw(std::runtime_error(sstr.str()));
    }

    addrptr = results;

    //multiple addresses may be returned work through them all
    while(addrptr)
    {
        Socket socket;

        WSABUF dataBuffer;
        dataBuffer.buf = 0;
        dataBuffer.len = 0;

        //attempt to connect to the address information
        if(SOCKET_ERROR
                == WSAConnect(socket.getSocket(), addrptr->ai_addr, (int) addrptr->ai_addrlen,
                        &dataBuffer, 0, 0, 0))
        {
            addrptr = addrptr->ai_next;
        }
        else
        {
            //successfully connected, we found a server to use
            mSocket = socket.getSocket();
            addrptr = NULL;
        }
    }

    freeaddrinfo(results);
    results = NULL;

    if(!mSocket || INVALID_SOCKET == mSocket)
    {
        std::stringstream sstr;
        sstr << "Failed to create client connection" << std::endl;
        throw(std::runtime_error(sstr.str()));
    }

    //wait for an acknowledgement that server is ready for identifier
    char ackBuff[4];
    ::recv(mSocket, ackBuff, sizeof(ackBuff), 0);
    //send over our connection information
    ::send(mSocket, onConnectMessage.c_str(), onConnectMessage.size(), 0);

    mDataBuffer.len = sizeof(mBuffer);
    mDataBuffer.buf = mBuffer;
    mOperation = Socket::OP_WAITING;
    createOverlap(this);
    mSendOverlap = new Overlap<Client>(this);

    mConnected = true;

    //queue our receive, we are now ready to go
    prepareClientToReceiveData();
}

const SOCKET Client::getSocket() const
{
    return mSocket;
}
const DWORD& Client::getBytes() const
{
    return mBytes;
}
const char* Client::getBuffer() const
{
    return mBuffer;
}
const WSABUF& Client::getDataBuffer() const
{
    return mDataBuffer;
}
const DWORD& Client::getOperation() const
{
    return mOperation;
}
const std::string& Client::getIdentifier() const
{
    return mIdentifier;
}

Callback* Client::getCallback()
{
    return mCallback;
}

const bool Client::isConnected() const
{
    return mConnected;
}

void Client::prepareClientToReceiveData() throw(std::runtime_error)
{
    DWORD flags = 0;
    DWORD nbBytesReceived = 0;

    /**
     * We need to queue an asynchronous receive, but since we're queue'ing
     * a receive, there exists the possibility that there is data ready to be
     * received. We need to check for i/o pending if WSARecv returns SOCKET_ERROR.
     */
    if(SOCKET_ERROR == WSARecv(mSocket, &mDataBuffer, 1, &nbBytesReceived, &flags, getOverlap(), 0))
    {
        int lastError = WSAGetLastError();

        if(WSA_IO_PENDING != lastError)
        {
            std::stringstream sstr;
            sstr << "Error prepping client socket for receive" << WSAGetLastError();
            throw(std::runtime_error(sstr.str()));
        }

        if(WSAECONNRESET == lastError) //server has shutdown, client no longer needed
        {
            delete this;
        }
    }
    else
    {
        /**
         * We received some data, pass it along to the callback, and attempt to queue another receive.
         */
        Callback* callback = getCallback();

        if(0 != callback)
            callback->receiveComplete(mIdentifier, mBuffer, nbBytesReceived);

        prepareClientToReceiveData();
    }
}

Client::~Client()
{
    closesocket(mSocket);
    mSocket = INVALID_SOCKET;
    delete mSendOverlap;
}

void Client::waitForEvents() throw(std::runtime_error)
{
    /**
     * Setup our list of handles that we're waiting for, one for the overlap used
     * to receive data, and one for the overlap used when sending data.
     */
    HANDLE handles[2];
    handles[0] = getOverlap()->hEvent;
    handles[1] = mSendOverlap->getOverlap()->hEvent;

    while(mConnected)
    {
        DWORD wait_rc = WSA_WAIT_IO_COMPLETION;

        /**
         * A handle can be signalted if i/o is not complete, so we want to wait
         * until i/o completes before we try to handle the signal.
         */
        while(WSA_WAIT_IO_COMPLETION == wait_rc && mConnected)
        {
            /**
             * Here is where we actually wait for all of our handles.
             */
            if(WSA_WAIT_FAILED
                    == (wait_rc = WSAWaitForMultipleEvents(2, handles, FALSE, WSA_INFINITE, TRUE)))
            {
                int error = WSAGetLastError();

                std::stringstream sstr;
                sstr << "WSAWaitForMultipleEvents Error " << WSAGetLastError();
                throw(std::runtime_error(sstr.str()));
            }
        }

        if(!mConnected)
            break;

        int index = (wait_rc - WSA_WAIT_EVENT_0);

        if(2 <= index)
        {
            throw(std::runtime_error("WSAWaitForMultipleEvents Returned invalid handle"));
        }

        DWORD nbBytes = 0;
        DWORD flags = 0;

        switch(index)
        {
            case 0:
                /**
                 * Get the results associated with our receive overlap. This indicates
                 * that a receive completed, and some number of bytes were received. FALSE
                 * indicates that if the receive isn't complete, we shouldn't wait. This will
                 * put us back in the top loop waiting for i/o completion.
                 */
                if(WSAGetOverlappedResult(mSocket, getOverlap(), &nbBytes, FALSE, &flags))
                {

                    Callback* callback = getCallback();

                    if(0 != callback)
                        callback->receiveComplete(mIdentifier, mBuffer, nbBytes);
                }

                break;

            case 1:
                /**
                 * Get the results associated with our send overlap. This indicates
                 * that a send completed, and some number of bytes were received. FALSE
                 * indicates that if the send isn't complete, we shouldn't wait. This will
                 * put us back in the top loop waiting for i/o completion.
                 */
                if(WSAGetOverlappedResult(mSocket, mSendOverlap->getOverlap(), &nbBytes, FALSE,
                        &flags))
                {
                    char* buffer = mSendBuffers.front();
                    mSendBuffers.pop();
                    delete[] buffer;

                    Callback* callback = getCallback();

                    if(0 != callback)
                        callback->sendComplete(mIdentifier, nbBytes);
                }

                break;

            default:
                break;
        }

        WSAResetEvent(handles[index]);
    }
}

void Client::disconnect() throw(std::runtime_error)
{

    if(mConnected)
    {
        mConnected = false;
        mCallback = 0;

        ::send(mSocket, NULL, 0, 0);

        WSASetEvent(getOverlap()->hEvent);

        /**
         * Close out our socket to sending and receiving. This doesn't actually send
         * or receive disconnection signals.
         */
WSASendDisconnect        (mSocket, 0);
        WSARecvDisconnect(mSocket, 0);

        Socket closedSocket(mSocket);
        closedSocket.closeSocket();
    }
}

void Client::sendDataToServer(const char* data, const unsigned int count) throw(std::runtime_error)
{
    char* buffer = new char[count];
    memcpy(buffer, data, count);
    mSendBuffers.push(buffer);
    WSABUF dataBuffer;
    DWORD flags = 0;
    DWORD nbBytes = 0;
    dataBuffer.len = count;
    dataBuffer.buf = (CHAR*) buffer;

    /**
     * Perform an asynchronous send, with no callback. This uses the overlapped
     * structure and its event handle to determine when the send is complete.
     */
    int iResult = WSASend(mSocket, &dataBuffer, 1, 0, flags, mSendOverlap->getOverlap(), 0);

    /**
     * Asynchronous send returns a SOCKET_ERROR if the send does not complete immediately.
     * Since this is asynchronous, it rarely completes immediately, so we then
     * need to check for WSA_IO_PENDING which shows that the send was successful, but
     * i/o is currently underway.
     */
    if(SOCKET_ERROR == iResult)
    {
        int lastError = WSAGetLastError();

        if(WSA_IO_PENDING != lastError)
        {
            throw(std::runtime_error("not io pending"));
        }
    }
    /**
     * If the send completed immediately, we need to use the overlapped structure
     * to determine how many bytes were sent.
     */
    else if(0 == iResult)
    {
        if(WSAGetOverlappedResult(mSocket, mSendOverlap->getOverlap(), &nbBytes, FALSE, 0))
        {
            //socket call completed
            Callback* callback = getCallback();

            if(0 != callback)
                callback->sendComplete(mIdentifier, nbBytes);
        }
    }

}

}
}
}

#endif