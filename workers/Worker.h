#pragma once

#include "workers/Platform.h"

#include "objects/HttpRequestToResponse.h"

#include <future>
#include <atomic>

namespace c11http {
namespace workers {

class WORKERS_API Worker {
public :
   typedef std::tuple<objects::HttpRequest, objects::HttpRequestToResponse> Work;
   Worker();
   ~Worker();

   void threadEntryPoint();

   void provideWork(const Work& work);

private:
   std::promise<Work> mPromiseToWork;
   std::atomic<bool> mShutdown;
};

}
}