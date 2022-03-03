#pragma once
extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
}
#include <memory>
#include <queue>
#include <string>

class Decoder {
 public:
  Decoder(int stream_index, std::shared_ptr<AVFormatContext>& format_context);
  ~Decoder();

  void init();

  bool getFrame(std::shared_ptr<AVPacket>& packet,
                std::queue<std::shared_ptr<AVFrame>>& frame_queue);

  int stream_index() const { return stream_index_; }

 private:
  int stream_index_;
  std::shared_ptr<AVFormatContext> format_context_;
  std::shared_ptr<AVCodecContext> codec_context_;
};