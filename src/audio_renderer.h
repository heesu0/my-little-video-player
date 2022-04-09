#pragma once
#ifdef __linux__
#include <SDL2/SDL.h>
#elif __APPLE__
#include <SDL.h>
#endif
extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
}
#include "audio_buffer.h"
#include <memory>
#include <mutex>
#include <queue>

class AudioRenderer {
 public:
  explicit AudioRenderer(std::shared_ptr<AVCodecContext>& codec_context,
                         AVRational time_base);
  ~AudioRenderer();

  void init();
  void start() const;
  void stop() const;
  void flush();

  void enqueueAudioBuffer(std::shared_ptr<AudioBuffer>& audio_buffer);
  int getAudioBuffer(uint8_t* audio_buffer, int buffer_size);
  uint64_t getAudioTime() const;

 private:
  std::shared_ptr<AVCodecContext> codec_context_;
  std::queue<std::shared_ptr<AudioBuffer>> buffer_queue_;
  std::mutex mutex_;
  AVRational time_base_;
  uint64_t audio_clock_;
  SDL_AudioDeviceID device_id_;
};
