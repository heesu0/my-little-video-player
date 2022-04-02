#include "timer.h"

Timer* Timer::instance_ = nullptr;

Timer* Timer::getInstance() {
  if (!instance_) {
    instance_ = new Timer();
  }
  return instance_;
}

Timer::Timer() : audio_time_(0) {}
