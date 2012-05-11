#include "workers/Worker.h"

namespace c11http {
namespace workers {

Worker::Worker() {

}

Worker::~Worker() {
   mPromiseToWork.set_value([](){mShutdown = true;});
}

void Worker::threadEntryPoint() {
   while(!shutdown) {
      std::future futureWork = mPromiseToWork.get_future();
      futureWork.wait();
      std::function func = futureWork.get();
      func();
   }
}

void Worker::fulfillPromise(const std::function& func) {
   mPromiseToWork.set_value(func);
}