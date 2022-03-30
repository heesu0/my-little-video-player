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
#include <future>
#include <memory>
#include <mutex>
#include <queue>
#include <atomic>

class VideoRenderer {
 public:
  explicit VideoRenderer(std::shared_ptr<AVCodecContext>& codec_context);
  ~VideoRenderer();

  void init();
  void run();
  void start();
  void stop();

  void enqueueFrame(std::shared_ptr<AVFrame>& frame);
  void renderFrame(std::shared_ptr<AVFrame>& frame);

 private:
  std::shared_ptr<SDL_Window> window_;
  std::shared_ptr<SDL_Renderer> renderer_;
  std::shared_ptr<SDL_Texture> texture_;
  std::shared_ptr<AVCodecContext> codec_context_;
  std::queue<std::shared_ptr<AVFrame>> frame_queue_;
  std::atomic<bool> running_;
  std::future<void> task_;
  std::mutex mutex_;
};
