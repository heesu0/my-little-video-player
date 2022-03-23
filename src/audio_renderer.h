#pragma once
extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
}
#include "SDL.h"
#include "audio_buffer.h"
#include <memory>
#include <mutex>
#include <queue>

class AudioRenderer {
 public:
  explicit AudioRenderer(std::shared_ptr<AVCodecContext>& codec_context);
  ~AudioRenderer();

  void init();

  void enqueueAudioBuffer(std::shared_ptr<AudioBuffer>& audio_buffer);
  int getAudioBuffer(uint8_t* audio_buffer, int buffer_size);

 private:
  std::shared_ptr<AVCodecContext> codec_context_;
  std::queue<std::shared_ptr<AudioBuffer>> buffer_queue_;
  std::mutex mutex_;
};
