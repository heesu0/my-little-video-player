#pragma once
extern "C" {
#include <libavformat/avformat.h>
}
#include <memory>
#include <string>

class Demuxer {
 public:
  explicit Demuxer(std::string filename);
  ~Demuxer();

  void init();
  void seek(uint64_t time);

  bool getPacket(std::shared_ptr<AVPacket>& packet);
  void printFileInfo();

  int video_stream_index() const { return video_stream_index_; }
  int audio_stream_index() const { return audio_stream_index_; }
  std::shared_ptr<AVFormatContext>& format_context() { return format_context_; }

 private:
  std::shared_ptr<AVFormatContext> format_context_;
  std::string filename_;
  int video_stream_index_;
  int audio_stream_index_;
};
