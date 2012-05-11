#pragma once

#include <future>
#include <functional>

namespace c11http {
namespace workers {

template<class FuncType>
class Worker {
public :
   Worker() {

   }
   ~Worker() {
      mShutdown = true;
      mPromiseToWork.set_value(FuncType());
   }

   void threadEntryPoint() {
      while(!mShutdown) {
         auto ftr = mPromiseToWork.get_future();
         ftr.wait();
         FuncType func = ftr.get();
         func();
      }
   }

   void provideWork(const FuncType& func);

private:
   std::promise<FuncType> mPromiseToWork;
   std::atomic mShutdown;
};

}
}