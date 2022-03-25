#pragma once
#ifdef __linux__
#include <SDL2/SDL.h>
#elif __APPLE__
#include <SDL.h>
#endif
extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
}
#include <memory>

class VideoRenderer {
 public:
  explicit VideoRenderer(std::shared_ptr<AVCodecContext>& codec_context);
  ~VideoRenderer();

  void init();

  void renderFrame(std::shared_ptr<AVFrame>& frame);

 private:
  std::shared_ptr<AVCodecContext> codec_context_;
  std::shared_ptr<SDL_Window> window_;
  std::shared_ptr<SDL_Renderer> renderer_;
  std::shared_ptr<SDL_Texture> texture_;
};
