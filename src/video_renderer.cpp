#include "video_renderer.h"

#include "log.h"
#include "timer.h"

#include <chrono>
#include <thread>

VideoRenderer::VideoRenderer(const std::shared_ptr<AVCodecContext>& codec_context, AVRational time_base)
    : codec_context_(codec_context), time_base_(time_base)
{
}

void VideoRenderer::Init()
{
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER))
    {
        LOG::ERROR_FROM_SDL();
    }

    window_ = std::shared_ptr<SDL_Window>(SDL_CreateWindow("my-little-video-player", SDL_WINDOWPOS_UNDEFINED,
                                                           SDL_WINDOWPOS_UNDEFINED, codec_context_->width,
                                                           codec_context_->height, SDL_WINDOW_RESIZABLE),
                                          [](SDL_Window* window) { SDL_DestroyWindow(window); });
    if (!window_)
    {
        LOG::ERROR_FROM_SDL();
    }

    renderer_ = std::shared_ptr<SDL_Renderer>(
        SDL_CreateRenderer(window_.get(), -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC),
        [](SDL_Renderer* renderer) { SDL_DestroyRenderer(renderer); });
    if (!renderer_)
    {
        LOG::ERROR_FROM_SDL();
    }

    texture_ = std::shared_ptr<SDL_Texture>(SDL_CreateTexture(renderer_.get(), SDL_PIXELFORMAT_YV12,
                                                              SDL_TEXTUREACCESS_STREAMING, codec_context_->width,
                                                              codec_context_->height),
                                            [](SDL_Texture* texture) { SDL_DestroyTexture(texture); });
    if (!texture_)
    {
        LOG::ERROR_FROM_SDL();
    }

    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");
    SDL_RenderSetLogicalSize(renderer_.get(), codec_context_->width, codec_context_->height);
    SDL_SetRenderDrawColor(renderer_.get(), 0, 0, 0, 255);
    SDL_RenderClear(renderer_.get());
    SDL_RenderPresent(renderer_.get());

    task_ = std::async(std::launch::async, &VideoRenderer::process, this);
}

void VideoRenderer::Start()
{
    running_ = true;
    task_ = std::async(std::launch::async, &VideoRenderer::process, this);
}

void VideoRenderer::Stop()
{
    running_ = false;
    task_.get();
}

void VideoRenderer::Flush()
{
    Stop();
    {
        std::lock_guard<std::mutex> lock(mutex_);
        frame_queue_ = std::queue<std::shared_ptr<AVFrame>>();
    }
    Start();
}

void VideoRenderer::EnqueueFrame(const std::shared_ptr<AVFrame>& frame)
{
    std::lock_guard<std::mutex> lock(mutex_);
    frame_queue_.push(frame);
}

void VideoRenderer::RenderFrame(const std::shared_ptr<AVFrame>& frame) const
{
    SDL_UpdateYUVTexture(texture_.get(), nullptr, frame->data[0], frame->linesize[0], frame->data[1],
                         frame->linesize[1], frame->data[2], frame->linesize[2]);
    SDL_RenderClear(renderer_.get());
    SDL_RenderCopy(renderer_.get(), texture_.get(), nullptr, nullptr);
    SDL_RenderPresent(renderer_.get());
}

void VideoRenderer::process()
{
    running_ = true;
    while (running_)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        std::lock_guard<std::mutex> lock(mutex_);

        if (!frame_queue_.empty())
        {
            std::shared_ptr<AVFrame> frame = frame_queue_.front();
            if (getVideoTime(frame) + 200 < Timer::GetInstance()->GetAudioTime())
            {
                frame_queue_.pop();
            }
            else if (getVideoTime(frame) < Timer::GetInstance()->GetAudioTime())
            {
                RenderFrame(frame);
                frame_queue_.pop();
            }
        }
    }
}

uint64_t VideoRenderer::getVideoTime(const std::shared_ptr<AVFrame>& frame) const
{
    return static_cast<uint64_t>(av_q2d(time_base_) * frame->pts * 1000);
}
