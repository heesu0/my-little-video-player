#pragma once
extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswresample/swresample.h>
}
#include <memory>

class AudioResampler {
 public:
  explicit AudioResampler(std::shared_ptr<AVCodecContext>& codec_context);
  ~AudioResampler();

  void init();

  int resampleFrame(std::shared_ptr<AVFrame>& input_frame,
                    uint8_t* output_buffer);

 private:
  std::shared_ptr<AVCodecContext> codec_context_;
  std::shared_ptr<SwrContext> swr_context_;
};
