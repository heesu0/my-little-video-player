#pragma once
extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/imgutils.h>
#include <libswscale/swscale.h>
}
#include <memory>

class VideoConverter {
 public:
  explicit VideoConverter(std::shared_ptr<AVCodecContext>& codec_context);
  ~VideoConverter();

  void init();

  void convertFrame(std::shared_ptr<AVFrame>& input_frame,
                std::shared_ptr<AVFrame>& output_frame);

 private:
  std::shared_ptr<AVCodecContext> codec_context_;
  std::shared_ptr<SwsContext> sws_context_;
};
