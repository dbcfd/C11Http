#include <thread>

#include "tcp/Client.h"
#include "tcp/Server.h"

#pragma warning(disable:4251)
#include <gtest/gtest.h>

class ClientThread {
public:
	ClientThread() {
		mClient = new c11http::tcp::Client("127.0.0.1", 80, "");
		mThread = new std::thread(&c11http::tcp::Client::waitForEvents, mClient);
	}

	~ClientThread() {
		mClient->disconnect();
		mThread->join();
		delete mClient;
		delete mThread;
	}
private:
	c11http::tcp::Client* mClient;
	std::thread* mThread;
};

class ServerThread {
public:
	ServerThread() {
		mServer = new c11http::tcp::Server(80);
		mThread = new std::thread(&c11http::tcp::Server::waitForEvents, mServer);
	}
private:
	c11http::tcp::Server* mServer;
	std::thread* mThread;
};

TEST(TCP_TEST, TEST_CONNECTIONS)
{
	using namespace c11http::tcp;
	
}