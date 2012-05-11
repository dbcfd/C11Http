#include "workers/Worker.h"

#include "objects/HttpRequest.h"
#include "objects/HttpResponse.h"

namespace c11http {
namespace workers {

Worker::Worker() {

}

Worker::~Worker() {
   auto reqToResp = [this](objects::HttpRequest) -> objects::HttpResponse {
      mShutdown = true;
      return objects::HttpResponse();
   };
   Work work = std::tuple<objects::HttpRequest, objects::HttpRequestToResponse>(objects::HttpRequest(), reqToResp);
   mPromiseToWork.set_value(work);
}

void Worker::threadEntryPoint() {
   while(!mShutdown) {
      std::future<Work> futureWork = mPromiseToWork.get_future();
      futureWork.wait();
      Work work = futureWork.get();
      objects::HttpRequestToResponse reqToResp = std::get<1>(work);
      reqToResp(std::get<0>(work));
   }
}

void Worker::provideWork(const Worker::Work& work) {
   mPromiseToWork.set_value(work);
}

}
}