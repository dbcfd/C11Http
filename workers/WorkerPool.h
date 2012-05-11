#pragma once

#include "workers/Platform.h"
#include "workers/Worker.h"

#include "objects/HttpRequestToResponse.h"

#include <vector>
#include <thread>
#include <atomic>

namespace c11http {
namespace workers {

class WORKERS_API WorkerPool {
public:
   WorkerPool(const int nbWorkers);
   ~WorkerPool();

   void addWork(const Worker::Work& work);
private:
   std::vector<Worker*> mWorkers;
   std::vector<std::thread> mThreads;
   size_t mCurrentWorker;
   std::mutex mMutex;
};

}
}

