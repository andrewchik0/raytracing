#include "thread_pool.h"

namespace raytracing::utils
{
  void thread_pool::start()
  {
    mShouldTerminate = false;
    for (size_t i = 0; i < mThreadCount; i++)
      mThreads.emplace_back(std::thread(&thread_pool::thread_loop, this));
  }

  void thread_pool::thread_loop()
  {
    while (true)
    {
      std::function<void()> job;

      // Create scope with locking to access queue
      {
        // Lock the queue
        std::unique_lock<std::mutex> lock(mQueueMutex);

        // Wait until some work will appear
        mMutexCondition.wait(lock, [this] {
          return !mJobs.empty() || mShouldTerminate;
        });

        if (mShouldTerminate)
          return;

        job = mJobs.front();
        mJobs.pop();
      }

      job();
    }
  }

  thread_pool::thread_pool()
      : thread_pool(std::thread::hardware_concurrency() - 1)
  {}

  thread_pool::thread_pool(const uint32_t threadCount)
    : mThreadCount(threadCount)
  {
    mThreads.resize(mThreadCount);
    start();
  }

  thread_pool::~thread_pool()
  {
    terminate();
  }

  void thread_pool::terminate()
  {
    {
      std::unique_lock<std::mutex> lock(mQueueMutex);
      mShouldTerminate = true;
    }
    mMutexCondition.notify_all();
    for (std::thread& thread : mThreads)
    {
      if (thread.joinable())
        thread.join();
    }
    mThreads.clear();
  }

  void thread_pool::restart()
  {
    resize(mThreadCount);
  }

  void thread_pool::resize(uint32_t threadCount)
  {
    terminate();
    mThreadCount = threadCount;
    start();
  }

  void thread_pool::send_task(const std::function<void()>& job)
  {
    {
      std::unique_lock<std::mutex> lock(mQueueMutex);
      mJobs.push(job);
    }
    mMutexCondition.notify_one();
  }

  bool thread_pool::is_busy()
  {
    bool bPoolBusy;
    {
      std::unique_lock<std::mutex> lock(mQueueMutex);
      bPoolBusy = !mJobs.empty();
    }
    return bPoolBusy;
  }
}
