#ifndef __TIMER_H__
#define __TIMER_H__

#include <cstdint>
#include <mutex>

class Timer
{
public:
    static Timer* GetInstance()
    {
        static Timer instance;
        return &instance;
    }

    uint64_t GetAudioTime() const
    {
        std::lock_guard<std::mutex> lock(mutex_);
        return audio_time_;
    };

    void SetAudioTime(uint64_t audio_time)
    {
        std::lock_guard<std::mutex> lock(mutex_);
        audio_time_ = audio_time;
    };

private:
    Timer() = default;

    mutable std::mutex mutex_;
    uint64_t audio_time_ = 0;
};

#endif
