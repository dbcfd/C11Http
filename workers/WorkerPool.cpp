#include "workers/WorkerPool.h"
#include "workers/Worker.h"

namespace c11http {
namespace workers {

WorkerPool::WorkerPool(const int nbWorkers) {
   if(nbWorkers <= 0) throw(std::runtime_error("Number of workers must be greater than 0"));
   mWorkers.reserve(nbWorkers);
   for(int i = 0; i < nbWorkers; ++i) {
      mWorkers.push_back(new Worker());
   }
   mThreads.reserve(nbWorkers);
   std::for_each(mWorkers.begin(), mWorkers.end(), [this](Worker* worker) {
      mThreads.push_back(std::thread(&Worker::threadEntryPoint, worker));
   });
   mCurrentWorker = 0;
      
}

WorkerPool::~WorkerPool() {
   std::for_each(mWorkers.begin(), mWorkers.end(), [this](Worker* worker) {
      delete worker;
   });
   std::for_each(mThreads.begin(), mThreads.end(), [this](std::thread& thread) {
      thread.join();
   });
}

void WorkerPool::addWork(const Worker::Work& work) {
   int worker;
   {
      std::lock_guard<std::mutex> lock(mMutex);
      worker = mCurrentWorker++;
      if(mCurrentWorker >= mWorkers.size()) mCurrentWorker = 0;
   }
   mWorkers[worker]->provideWork(work);

}

}
}