#include "tcp/Server.h"

#ifdef WINDOWS
#include "tcp/windows/Server.h"
#include "tcp/windows/Callback.h"
#else
#include "tcp/posix/Server.h"
#include "tcp/posix/Callback.h"
#endif

namespace c11http {
   namespace tcp {

#ifdef WINDOWS
      class Server::PlatformCallback : public windows::Callback {
#else
      class Server::PlatformCallback : public posix::Callback {
#endif
      public:
         PlatformCallback(Server* server) : mServer(server) 
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
            mServer->receiveComplete(identifier, data, count);
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
         Server* mServer;
      };

      class Server::PlatformServer {
      public:
         PlatformServer(Server::PlatformCallback* callback, const unsigned int port) {
#ifdef WINDOWS
            mServer = new windows::Server(callback, port);
#else
            mServer = new posix::Server(callback, port);
#endif
         }

         void waitForEvents()
         {
            mServer->waitForEvents();
         }

         void send(const char* data, const unsigned int count, const std::string& identifier)
         {
            mServer->send(data, count, identifier);
         }

         void broadcast(const char* data, const unsigned int count)
         {
            mServer->broadcast(data, count);
         }

         void shutdown()
         {
            mServer->shutdown();
         }

      private:
#ifdef WINDOWS
         windows::Server* mServer;
#else
         posix::Server* mServer;
#endif
      };

      Server::Server(const unsigned int port)
      {
         mCallback = new PlatformCallback(this);
         mServer = new PlatformServer(mCallback, port);
      }

      Server::~Server()
      {
         mServer->shutdown();
         delete mServer;
         mServer = 0;
         delete mCallback;
         mCallback = 0;
      }

      void Server::waitForEvents() {
         mServer->waitForEvents();
      }
      void Server::send(const char* data, const unsigned int count, const std::string& identifier) {
         mServer->send(data, count, identifier);
      }
      void Server::broadcast(const char* data, const unsigned int count) {
         mServer->broadcast(data, count);
      }
      void Server::shutdown() {
         mServer->shutdown();
      }

   }
}
