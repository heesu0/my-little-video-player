#pragma once
#include <cstdint>

class Timer {
 public:
  static Timer* getInstance();

  uint64_t getAudioTime() const { return audio_time_; };
  void setAudioTime(uint64_t audio_time) { audio_time_ = audio_time; };

 private:
  Timer();

  static Timer* instance_;
  uint64_t audio_time_;
};
