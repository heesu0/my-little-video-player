#include "demuxer.h"
#include "log.h"
#include <iostream>
#include <utility>

Demuxer::Demuxer(std::string filename)
    : filename_(std::move(filename)), video_stream_index_(-1),
      audio_stream_index_(-1) {}

Demuxer::~Demuxer() = default;

void Demuxer::init() {
  AVFormatContext* format_context = nullptr;
  int ret = avformat_open_input(&format_context, filename_.c_str(), nullptr,
                                nullptr);
  if (ret < 0) LOG::ERROR_FROM_FFMPEG(ret);

  format_context_ = std::shared_ptr<AVFormatContext>(
          format_context, [](AVFormatContext* format_context) {
            avformat_close_input(&format_context);
            std::cout << "AVFormat close input" << std::endl;
          });
  ret = avformat_find_stream_info(format_context_.get(), nullptr);
  if (ret < 0) LOG::ERROR_FROM_FFMPEG(ret);

  video_stream_index_ = av_find_best_stream(
          format_context_.get(), AVMEDIA_TYPE_VIDEO, -1, -1, nullptr, 0);
  if (video_stream_index_ == AVERROR_STREAM_NOT_FOUND) {
    LOG::ERROR_FROM_FFMPEG(video_stream_index_);
  }

  audio_stream_index_ = av_find_best_stream(
          format_context_.get(), AVMEDIA_TYPE_AUDIO, -1, -1, nullptr, 0);
  if (audio_stream_index_ == AVERROR_STREAM_NOT_FOUND) {
    LOG::ERROR_FROM_FFMPEG(audio_stream_index_);
  }
}

bool Demuxer::getPacket(std::shared_ptr<AVPacket>& packet) {
  AVPacket* av_packet = av_packet_alloc();
  if (av_read_frame(format_context_.get(), av_packet) >= 0) {
    packet = std::shared_ptr<AVPacket>(
            av_packet, [](AVPacket* packet) { av_packet_free(&packet); });
    return true;
  } else {
    std::cout << "End of frame" << std::endl;
    av_packet_free(&av_packet);
    return false;
  }
}

void Demuxer::printFileInfo() {
  av_dump_format(format_context_.get(), 0, filename_.c_str(), 0);
}
