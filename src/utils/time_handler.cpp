#include <iostream>
#include <chrono>
#include <iomanip>

#include "time_handler.h"

namespace raytracing::utils {

  uint64_t time_handler::get_time_in_ms()
  {
    return get_time<std::chrono::milliseconds>();
  }

  uint64_t time_handler::get_time_in_ns()
  {
    return get_time<std::chrono::nanoseconds>();
  }

  template<typename T>
  uint64_t time_handler::get_time()
  {
    using namespace std::chrono;
    return duration_cast<T>(high_resolution_clock::now().time_since_epoch()).count();
  }

  time_handler::time_handler()
  {
    mStartTime_ns = get_time_in_ns();
    mFpsCounterTime = 0.0;
    mDeltaFrameCount = 0;
    mFps = 0.0;
    mTimeSinceStart_ns = 0;
    mTimeSinceStart = 0.0;
    mAllZonesTime = 0;

    mZones.resize(ZONES_COUNT);
  }

  void time_handler::tick()
  {
    mDeltaTime_ns = get_time_in_ns() - mTimeSinceStart_ns - mStartTime_ns;
    mTimeSinceStart_ns = get_time_in_ns() - mStartTime_ns;
    mFpsCounterTime += (double)mDeltaTime_ns / 1e9;
    mDeltaFrameCount++;
    mDeltaTime = (double)mDeltaTime_ns / 1e9;
    mTimeSinceStart = mTimeSinceStart_ns / 1.0e9;

    if (mFpsCounterTime >= mFpsCounterInterval)
    {
      mFps = mDeltaFrameCount / (double)mFpsCounterTime;
      mFpsCounterTime = 0.0;
      mDeltaFrameCount = 0;
    }

    mAllZonesTime = 0;
    mZones.clear();
    mZones.push_back(time_zone{"start", 0});
  }

  void time_handler::mark_zone(const char* zoneName)
  {
    mZones.push_back({zoneName, get_time_in_ns() - mStartTime_ns - mTimeSinceStart_ns - mAllZonesTime});
    mAllZonesTime += mZones[mZones.size() - 1].consumedTime;
  }
}