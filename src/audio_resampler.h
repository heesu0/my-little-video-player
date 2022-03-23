#pragma once
extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswresample/swresample.h>
}
#include "audio_buffer.h"
#include <memory>

class AudioResampler {
 public:
  explicit AudioResampler(std::shared_ptr<AVCodecContext>& codec_context);
  ~AudioResampler();

  void init();

  void resampleFrame(std::shared_ptr<AVFrame>& input_frame,
                     std::shared_ptr<AudioBuffer>& output_buffer);

 private:
  std::shared_ptr<AVCodecContext> codec_context_;
  std::shared_ptr<SwrContext> swr_context_;
};
