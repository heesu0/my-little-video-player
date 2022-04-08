#include "video_renderer.h"
#include "log.h"
#include "timer.h"

#include <chrono>
#include <thread>

VideoRenderer::VideoRenderer(std::shared_ptr<AVCodecContext>& codec_context,
                             AVRational time_base)
    : codec_context_(codec_context), running_(false), time_base_(time_base) {}

VideoRenderer::~VideoRenderer() = default;

void VideoRenderer::init() {
  if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER)) {
    LOG::ERROR_FROM_SDL();
  }

  window_ = std::shared_ptr<SDL_Window>(
          SDL_CreateWindow("my-little-video-player", SDL_WINDOWPOS_UNDEFINED,
                           SDL_WINDOWPOS_UNDEFINED, codec_context_->width,
                           codec_context_->height, SDL_WINDOW_RESIZABLE),
          [](SDL_Window* window) { SDL_DestroyWindow(window); });
  if (!window_) LOG::ERROR_FROM_SDL();

  renderer_ = std::shared_ptr<SDL_Renderer>(
          SDL_CreateRenderer(window_.get(), -1,
                             SDL_RENDERER_ACCELERATED |
                                     SDL_RENDERER_PRESENTVSYNC),
          [](SDL_Renderer* renderer) { SDL_DestroyRenderer(renderer); });
  if (!renderer_) LOG::ERROR_FROM_SDL();

  texture_ = std::shared_ptr<SDL_Texture>(
          SDL_CreateTexture(renderer_.get(), SDL_PIXELFORMAT_YV12,
                            SDL_TEXTUREACCESS_STREAMING, codec_context_->width,
                            codec_context_->height),
          [](SDL_Texture* texture) { SDL_DestroyTexture(texture); });
  if (!texture_) LOG::ERROR_FROM_SDL();

  SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");
  SDL_RenderSetLogicalSize(renderer_.get(), codec_context_->width,
                           codec_context_->height);

  SDL_SetRenderDrawColor(renderer_.get(), 0, 0, 0, 255);
  SDL_RenderClear(renderer_.get());
  SDL_RenderPresent(renderer_.get());

  task_ = std::async(std::launch::async, &VideoRenderer::run, this);
}

void VideoRenderer::run() {
  running_ = true;
  while (running_) {
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
    mutex_.lock();
    if (!frame_queue_.empty()) {
      std::shared_ptr<AVFrame> frame = frame_queue_.front();
      if (getTimestamp(frame) + 200 < Timer::getInstance()->getAudioTime()) {
        frame_queue_.pop();
      } else if (getTimestamp(frame) < Timer::getInstance()->getAudioTime()) {
        renderFrame(frame);
        frame_queue_.pop();
      }
    }
    mutex_.unlock();
  }
}

void VideoRenderer::start() {
  running_ = true;
  task_ = std::async(std::launch::async, &VideoRenderer::run, this);
}

void VideoRenderer::stop() {
  running_ = false;
  task_.get();
}

void VideoRenderer::flush() {
  stop();
  mutex_.lock();
  frame_queue_ = std::queue<std::shared_ptr<AVFrame>>();
  mutex_.unlock();
  start();
}

void VideoRenderer::enqueueFrame(std::shared_ptr<AVFrame>& frame) {
  mutex_.lock();
  frame_queue_.push(frame);
  mutex_.unlock();
}

void VideoRenderer::renderFrame(std::shared_ptr<AVFrame>& frame) {
  SDL_UpdateYUVTexture(texture_.get(), nullptr, frame->data[0],
                       frame->linesize[0], frame->data[1], frame->linesize[1],
                       frame->data[2], frame->linesize[2]);
  SDL_RenderClear(renderer_.get());
  SDL_RenderCopy(renderer_.get(), texture_.get(), nullptr, nullptr);
  SDL_RenderPresent(renderer_.get());
}

uint64_t VideoRenderer::getTimestamp(std::shared_ptr<AVFrame>& frame) {
  return static_cast<uint64_t>(av_q2d(time_base_) * frame->pts * 1000);
}
