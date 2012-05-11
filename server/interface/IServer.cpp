#include "tcp/IServer.h"

namespace c11http {
namespace tcp {

IServer::IServer(WorkerPool* pool) : mPool(pool) {
   
}

void IServer::addWork(std::function func) {
   mPool->addWork(func);
}