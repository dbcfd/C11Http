#pragma once

#include <queue>

#include "client/windows/Overlap.h"

namespace c11http {
namespace client {
namespace windows {

/**
 * Winsock2 implementation of a client, which is a connection to some server. This class is implemented asynchronously
 * and will not create any additional threads.
 */
class CLIENT_WINDOWS_API Client : public IClient, public Overlap<Client>
{
public:
	Client(const IClient::ClientResponseCallback& callback);
	~Client();

	virtual objects::HttpResponse sendRequestToServer(const objects::HttpRequest& req);
   virtual void receiveResponseFromServer(const objects::HttpResponse& resp);
   virtual void disconnect();

protected:
   virtual void performServerConnection(const std::string& ip, const unsigned int port);

private:
	SOCKET mSocket;
	DWORD mBytes;
	char mBuffer[MAX_BUFFER_SIZE];
	WSABUF mDataBuffer;
	Overlap<Client>* mSendOverlap;
};

}
}
}

