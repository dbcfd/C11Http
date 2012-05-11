#include <thread>
#include <chrono>

#include "tcp/Client.h"
#include "tcp/Server.h"

#pragma warning(disable:4251)
#include <gtest/gtest.h>

class ClientThread {
public:
   class MyClient : public c11http::tcp::Client {
   public :
      MyClient() : c11http::tcp::Client("127.0.0.1", 80, "TestClient") {

      }
      virtual void receiveComplete(const std::string& identifier, const char* data, const unsigned int count) {
         EXPECT_TRUE(std::string(data, count) == std::string("This is data from the server"));
      }
   };
   ClientThread() {
      mClient = new MyClient();
      mThread = new std::thread(&c11http::tcp::Client::waitForEvents, mClient);
   }

   ~ClientThread() {
      mClient->disconnect();
      mThread->join();
      delete mClient;
      delete mThread;
   }

   void runTest() {
      std::string testString("This is data from the client");
      mClient->sendDataToServer(testString.c_str(), testString.size());
   }
private:
   MyClient* mClient;
   std::thread* mThread;
};

class ServerThread {
public:
   class MyServer : public c11http::tcp::Server {
   public:
      MyServer() : c11http::tcp::Server(80) {

      }
      virtual void receiveComplete(const std::string& identifier, const char* data, const unsigned int count) {
         EXPECT_TRUE(std::string(data,count) == std::string("This is data from the client"));
      }
   };

   ServerThread() {
      mServer = new MyServer();
      mThread = new std::thread(&c11http::tcp::Server::waitForEvents, mServer);
   }

   ~ServerThread() {
      mServer->shutdown();
      mThread->join();
      delete mServer;
      delete mThread;
   }

   void runTest() {
      std::string testString("This is data from the server");
      mServer->broadcast(testString.c_str(), testString.size());
   }
private:
   MyServer* mServer;
   std::thread* mThread;
};

TEST(TCP_TEST, TEST_CONNECTIONS)
{
   using namespace c11http::tcp;

   ServerThread server;
   std::chrono::milliseconds startupWaitTime(25);
   std::this_thread::sleep_for(startupWaitTime); //wait for server to start listening for clients

   ClientThread client;
   std::this_thread::sleep_for(startupWaitTime); //wait for client to connect to server

   server.runTest();
   client.runTest();

   std::this_thread::sleep_for(startupWaitTime * 5); //give our test time to run
}