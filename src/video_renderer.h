#ifndef __VIDEO_RENDERER_H__
#define __VIDEO_RENDERER_H__

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
}

#include <SDL.h>
#include <atomic>
#include <future>
#include <memory>
#include <mutex>
#include <queue>

class VideoRenderer
{
public:
    explicit VideoRenderer(const std::shared_ptr<AVCodecContext>& codec_context, AVRational time_base);
    ~VideoRenderer() = default;

    void Init();
    void Start();
    void Stop();
    void Flush();
    void EnqueueFrame(const std::shared_ptr<AVFrame>& frame);
    void RenderFrame(const std::shared_ptr<AVFrame>& frame) const;

private:
    void process();
    uint64_t getVideoTime(const std::shared_ptr<AVFrame>& frame) const;

    std::shared_ptr<SDL_Window> window_;
    std::shared_ptr<SDL_Renderer> renderer_;
    std::shared_ptr<SDL_Texture> texture_;

    std::shared_ptr<AVCodecContext> codec_context_;
    std::queue<std::shared_ptr<AVFrame>> frame_queue_;

    std::atomic<bool> running_{false};
    std::future<void> task_;
    std::mutex mutex_;

    AVRational time_base_;
};

#endif
