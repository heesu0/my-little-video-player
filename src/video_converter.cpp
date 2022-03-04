#include "video_converter.h"
#include "log.h"

VideoConverter::VideoConverter(std::shared_ptr<AVCodecContext>& codec_context)
    : codec_context_(codec_context) {}

VideoConverter::~VideoConverter() = default;

void VideoConverter::init() {
  sws_context_ = std::shared_ptr<SwsContext>(
          sws_getContext(codec_context_->width, codec_context_->height,
                         codec_context_->pix_fmt, codec_context_->width,
                         codec_context_->height, AV_PIX_FMT_YUV420P,
                         SWS_BICUBIC, nullptr, nullptr, nullptr),
          [](SwsContext* sws_context) { sws_freeContext(sws_context); });
}

void VideoConverter::convertFrame(std::shared_ptr<AVFrame>& input_frame,
                                  std::shared_ptr<AVFrame>& output_frame) {
  AVFrame* frame = av_frame_alloc();
  int ret = av_image_alloc(frame->data, frame->linesize, codec_context_->width,
                           codec_context_->height, AV_PIX_FMT_YUV420P, 32);
  if (ret < 0) LOG::Error(ret);

  frame->pts = input_frame->pts;
  sws_scale(sws_context_.get(), input_frame->data, input_frame->linesize, 0,
            codec_context_->height, frame->data, frame->linesize);
  output_frame = std::shared_ptr<AVFrame>(frame, [](AVFrame* frame) {
    av_freep(&frame->data[0]);
    av_frame_free(&frame);
  });
}
