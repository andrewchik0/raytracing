#pragma once

#include <functional>
#include <future>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

namespace raytracing::utils
{
  class thread_pool
  {
  public:
    thread_pool();
    thread_pool(const thread_pool&) = delete;
    explicit thread_pool(uint32_t threadCount);

    ~thread_pool();

    void terminate();
    void resize(uint32_t threadCount);

    template <typename F, typename R = std::invoke_result_t<F>>
    [[nodiscard("send_task function should be used if future object is not needed")]] std::future<R> enqueue(const F& job)
    {
      std::shared_ptr<std::promise<R>> promise = std::make_shared<std::promise<R>>();
      {
        std::unique_lock<std::mutex> lock(mQueueMutex);
        mJobs.push([promise, &job]{
          if constexpr(std::is_void_v<R>)
          {
            job();
            promise->set_value();
          }
          else
            promise->set_value(job());
        });
      }
      mMutexCondition.notify_one();
      return promise->get_future();
    }

    void send_task(const std::function<void()>& job);
    bool is_busy();

  private:

    bool mShouldTerminate = false;
    std::vector<std::thread> mThreads;
    std::mutex mQueueMutex;
    std::condition_variable mMutexCondition;
    uint32_t mThreadCount;
    std::queue<std::function<void()>> mJobs;

    void start();
    void thread_loop();
  };
}