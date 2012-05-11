#pragma once

namespace c11http {
namespace tcp {

/**
 * Server interface. Platform specific implementations will implement this interface, allowing
 * common access to the platform servers
 */
class IServer {
public:
   IServer(WorkerPool* pool);

   virtual void send(IServerConnection* connection, const std::string& msg) = 0;
   virtual void waitForEvents() = 0;
   virtual void disconnect();

   void addWork(std::function func);
private:
   WorkerPool* mPool;
};

}
}