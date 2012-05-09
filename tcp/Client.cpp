#include "tcp/Client.h"

#ifdef WINDOWS
#include "tcp/windows/Client.h"
#include "tcp/windows/Callback.h"
#else
#include "tcp/posix/Client.h"
#include "tcp/posix/Callback.h"
#endif

namespace c11http {
namespace tcp {

#ifdef WINDOWS
class Client::PlatformCallback : public windows::Callback
#else
class Client::PlatformCallback : public posix::Callback
#endif
{
public:
	PlatformCallback(Client* client) : mClient(client)
	{

	}
	/**
     * A send to a connection has completed, with count bytes sent.
     */
    virtual void sendComplete(const std::string& identifier, const unsigned int count)
	{

	}
    /**
     * A send to a connection has failed, due to message.
     */
    virtual void sendFailed(const std::string& identifier, const std::string& message)
	{

	}
    /**
     * A receive from a connection has completed, with data of size count received.
     */
    virtual void receiveComplete(const std::string& identifier, const char* data, const unsigned int count)
	{
		mClient->receiveComplete(identifier, data, count);
	}
    /**
     * A connection has been established.
     */
    virtual void connected(const std::string& connectedTo) 
	{
		
	}
    /**
     * A connection has been terminated.
     */
    virtual void disconnected(const std::string& connectedTo)
	{
		
	}
private:
	Client* mClient;
};

class Client::PlatformClient {
public:
	PlatformClient(PlatformCallback* callback, const std::string& hostname, const unsigned int port, const std::string& onConnectMessage) {
#ifdef WINDOWS
		mClient = new windows::Client(callback, hostname, port, onConnectMessage);
#else
		mClient = new posix::Client(callback, hostname, port, onConnectMessage);
#endif
	}

    void waitForEvents()
	{
		mClient->waitForEvents();
	}
    
    void send(const char* data, const unsigned int count, const std::string& identifier)
	{
		mClient->sendDataToServer(data, count);
	}
    
    void disconnect()
	{
		mClient->disconnect();
	}

private:
#ifdef WINDOWS
	windows::Client* mClient;
#else
	posix::Client* mClient;
#endif
};

Client::Client(const std::string& hostname, const unsigned int port, const std::string& onConnectMessage)
{
	mCallback = new PlatformCallback(this);
	mClient = new PlatformClient(mCallback, hostname, port, onConnectMessage);
}

Client::~Client()
{
	mClient->disconnect();
	delete mClient;
	mClient = 0;
	delete mCallback;
	mCallback = 0;
}

void Client::receiveComplete(const std::string& identifier, const char* data, const unsigned int count)
{

}

}
}