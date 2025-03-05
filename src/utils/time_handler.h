#pragma once

namespace raytracing::utils
{
  struct time_zone
  {
    std::string name;
    uint64_t consumedTime;
  };

  class time_handler
  {
  public:
    double mDeltaTime = 0, mFps = 0, mTimeSinceStart = 0;

    std::vector<time_zone> mZones;
    uint64_t mAllZonesTime;

    time_handler();

    void tick();
    void mark_zone(const char* zoneName);

  private:
    // Time in nanoseconds
    uint64_t mTimeSinceStart_ns, mStartTime_ns, mDeltaTime_ns;

    // Time in seconds
    double mFpsCounterTime;

    static constexpr double mFpsCounterInterval = 0.25;

    // Frame counter between measurements
    int mDeltaFrameCount;

    static constexpr size_t ZONES_COUNT = 256;

    uint64_t get_time_in_ms();
    uint64_t get_time_in_ns();

    template<typename T>
    uint64_t get_time();
  };

}