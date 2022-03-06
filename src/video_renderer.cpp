#include "video_renderer.h"
#include "log.h"

#include <iostream>

VideoRenderer::VideoRenderer(std::shared_ptr<AVCodecContext>& codec_context)
    : codec_context_(codec_context) {}

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
}

void VideoRenderer::renderFrame(std::shared_ptr<AVFrame>& frame) {
  SDL_UpdateYUVTexture(texture_.get(), nullptr, frame->data[0],
                       frame->linesize[0], frame->data[1], frame->linesize[1],
                       frame->data[2], frame->linesize[2]);
  SDL_RenderClear(renderer_.get());
  SDL_RenderCopy(renderer_.get(), texture_.get(), nullptr, nullptr);
  SDL_RenderPresent(renderer_.get());
}
