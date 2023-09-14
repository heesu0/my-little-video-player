#include "video_player.h"

#include "audio_buffer.h"
#include "audio_renderer.h"
#include "audio_resampler.h"
#include "decoder.h"
#include "demuxer.h"
#include "timer.h"
#include "video_converter.h"
#include "video_renderer.h"

#include <SDL.h>
#include <algorithm>
#include <iostream>

namespace {

constexpr int MAX_RENDERER_QUEUE_SIZE = 20;
constexpr int MAX_DEMUXING_COUNT = 20;

enum EventType
{
    DEMUXING,
    VIDEO_DECODING,
    AUDIO_DECODING,
    VIDEO_CONVERTING,
    AUDIO_RESAMPLING,
    VIDEO_RENDERING,
    AUDIO_RENDERING,
    DEMUXING_COMPLETED,
    EVENT_COUNT
};

Uint32 SDL_EVENTS[EVENT_COUNT];

void REGIGSTER_SDL_EVENTS()
{
    for (int i = 0; i < EVENT_COUNT; ++i)
    {
        SDL_EVENTS[i] = SDL_RegisterEvents(i + 1);
    }
}

} // namespace

VideoPlayer::VideoPlayer(const std::string& filename) : filename_(filename)
{
}

void VideoPlayer::Init()
{
    demuxer_ = std::make_shared<Demuxer>(filename_);
    demuxer_->Init();

    video_decoder_ = std::make_shared<Decoder>(demuxer_->format_context(), demuxer_->video_stream_index());
    video_decoder_->Init();

    audio_decoder_ = std::make_shared<Decoder>(demuxer_->format_context(), demuxer_->audio_stream_index());
    audio_decoder_->Init();

    auto video_codec_context = video_decoder_->codec_context();
    auto audio_codec_context = audio_decoder_->codec_context();

    video_converter_ = std::make_shared<VideoConverter>(video_codec_context);
    video_converter_->Init();

    audio_resampler_ = std::make_shared<AudioResampler>(audio_codec_context);
    audio_resampler_->Init();

    auto video_time_base = demuxer_->format_context()->streams[video_decoder_->stream_index()]->time_base;
    auto audio_time_base = demuxer_->format_context()->streams[audio_decoder_->stream_index()]->time_base;

    video_renderer_ = std::make_shared<VideoRenderer>(video_codec_context, video_time_base);
    video_renderer_->Init();

    audio_renderer_ = std::make_shared<AudioRenderer>(audio_codec_context, audio_time_base);
    audio_renderer_->Init();

    REGIGSTER_SDL_EVENTS();
}

void VideoPlayer::Run()
{
    SDL_Event event;
    while (true)
    {
        SDL_PollEvent(&event);

        if (event.type == SDL_QUIT)
        {
            std::cout << "Press Quit" << std::endl;
            flushMediaQueue();
            SDL_Quit();
            break;
        }
        else if (event.type == SDL_KEYDOWN)
        {
            switch (event.key.keysym.sym)
            {
            case SDLK_p:
            case SDLK_SPACE:
                std::cout << "Press Space" << std::endl;
                if (is_running_)
                {
                    stop();
                }
                else
                {
                    start();
                }
                break;

            case SDLK_LEFT:
                std::cout << "Press Reverse" << std::endl;
                seekPlayback(-(10 * 1000)); // mililsecond
                break;

            case SDLK_RIGHT:
                std::cout << "Press Forward" << std::endl;
                seekPlayback(10 * 1000); // mililsecond
                break;
            }
        }
        else if (event.type == SDL_EVENTS[DEMUXING])
        {
            task_queue_.emplace_back(std::async(std::launch::async, &VideoPlayer::demuxing, this));
        }
        else if (event.type == SDL_EVENTS[VIDEO_DECODING])
        {
            task_queue_.emplace_back(std::async(std::launch::async, &VideoPlayer::decodingVideo, this));
        }
        else if (event.type == SDL_EVENTS[AUDIO_DECODING])
        {
            task_queue_.emplace_back(std::async(std::launch::async, &VideoPlayer::decodingAudio, this));
        }
        else if (event.type == SDL_EVENTS[VIDEO_CONVERTING])
        {
            task_queue_.emplace_back(std::async(std::launch::async, &VideoPlayer::convertingVideo, this));
        }
        else if (event.type == SDL_EVENTS[AUDIO_RESAMPLING])
        {
            task_queue_.emplace_back(std::async(std::launch::async, &VideoPlayer::resamplingAudio, this));
        }
        else if (event.type == SDL_EVENTS[VIDEO_RENDERING])
        {
            task_queue_.emplace_back(std::async(std::launch::async, &VideoPlayer::renderingVideo, this));
        }
        else if (event.type == SDL_EVENTS[AUDIO_RENDERING])
        {
            task_queue_.emplace_back(std::async(std::launch::async, &VideoPlayer::renderingAudio, this));
        }
        else if (event.type == SDL_EVENTS[DEMUXING_COMPLETED])
        {
            is_demuxing_completed_ = true;
        }

        task_queue_.erase(std::remove_if(task_queue_.begin(), task_queue_.end(),
                                         [](auto& task) {
                                             return task.wait_for(std::chrono::milliseconds(10)) ==
                                                    std::future_status::ready;
                                         }),
                          task_queue_.end());

        if (!is_demuxing_completed_ && is_running_)
        {
            bool is_demuxing_requried = video_renderer_queue_.size() < MAX_RENDERER_QUEUE_SIZE ||
                                        audio_renderer_queue_.size() < MAX_RENDERER_QUEUE_SIZE;
            if (is_demuxing_requried && demuxing_count_ < MAX_DEMUXING_COUNT)
            {
                SDL_Event demuxing_event;
                demuxing_event.type = SDL_EVENTS[DEMUXING];
                demuxing_count_++;
                SDL_PushEvent(&demuxing_event);
            }
        }
    }

    for (auto& task : task_queue_)
    {
        task.get();
    }
}

void VideoPlayer::start()
{
    video_renderer_->Start();
    audio_renderer_->Start();
    is_running_ = true;
}

void VideoPlayer::stop()
{
    video_renderer_->Stop();
    audio_renderer_->Stop();
    is_running_ = false;
}

void VideoPlayer::seekPlayback(int seekDuration)
{
    flushMediaQueue();

    for (auto& task : task_queue_)
    {
        task.get();
    }
    task_queue_.clear();

    // Thread safe because all tasks are completed
    video_renderer_->Flush();
    audio_renderer_->Flush();
    video_decoder_->Flush();
    audio_decoder_->Flush();

    Timer::GetInstance()->SetAudioTime(Timer::GetInstance()->GetAudioTime() + seekDuration);
    demuxer_->Seek(Timer::GetInstance()->GetAudioTime());

    demuxing_count_ = 0;
    is_demuxing_completed_ = false;

    // Is it necessary?
    SDL_PumpEvents();
}

void VideoPlayer::demuxing()
{
    SDL_Event demuxing_event;
    std::lock_guard<std::mutex> lock(demuxer_mutex_);

    std::shared_ptr<AVPacket> packet;
    if (demuxer_->GetPacket(packet))
    {
        if (packet->stream_index == demuxer_->video_stream_index())
        {
            video_decoder_queue_.push(packet);
            demuxing_event.type = SDL_EVENTS[VIDEO_DECODING];
        }
        else if (packet->stream_index == demuxer_->audio_stream_index())
        {
            audio_decoder_queue_.push(packet);
            demuxing_event.type = SDL_EVENTS[AUDIO_DECODING];
        }
    }
    else
    {
        demuxing_event.type = SDL_EVENTS[DEMUXING_COMPLETED];
    }

    demuxing_count_--;
    SDL_PushEvent(&demuxing_event);
}

void VideoPlayer::decodingVideo()
{
    SDL_Event converting_event;
    std::lock_guard<std::mutex> lock(video_decoder_mutex_);

    std::shared_ptr<AVPacket> packet = video_decoder_queue_.front();
    if (packet)
    {
        std::queue<std::shared_ptr<AVFrame>> frame_queue;
        video_decoder_->GetFrame(packet, frame_queue);
        while (!frame_queue.empty())
        {
            std::shared_ptr<AVFrame> frame = frame_queue.front();
            video_converter_queue_.push(frame);
            frame_queue.pop();

            converting_event.type = SDL_EVENTS[VIDEO_CONVERTING];
            SDL_PushEvent(&converting_event);
        }
    }
}

void VideoPlayer::decodingAudio()
{
    SDL_Event resampling_event;
    std::lock_guard<std::mutex> lock(audio_decoder_mutex_);

    std::shared_ptr<AVPacket> packet = audio_decoder_queue_.front();
    if (packet)
    {
        std::queue<std::shared_ptr<AVFrame>> frame_queue;
        audio_decoder_->GetFrame(packet, frame_queue);
        while (!frame_queue.empty())
        {
            std::shared_ptr<AVFrame> frame = frame_queue.front();
            audio_resampler_queue_.push(frame);
            frame_queue.pop();

            resampling_event.type = SDL_EVENTS[AUDIO_RESAMPLING];
            SDL_PushEvent(&resampling_event);
        }
    }
}

void VideoPlayer::convertingVideo()
{
    SDL_Event rendering_event;
    std::lock_guard<std::mutex> lock(video_converter_mutex_);

    std::shared_ptr<AVFrame> frame = video_converter_queue_.front();
    if (frame)
    {
        std::shared_ptr<AVFrame> converted_frame;
        video_converter_->ConvertFrame(frame, converted_frame);
        video_renderer_queue_.push(converted_frame);

        rendering_event.type = SDL_EVENTS[VIDEO_RENDERING];
        SDL_PushEvent(&rendering_event);
    }
}

void VideoPlayer::resamplingAudio()
{
    SDL_Event rendering_event;
    std::lock_guard<std::mutex> lock(audio_resampler_mutex_);

    std::shared_ptr<AVFrame> frame = audio_resampler_queue_.front();
    if (frame)
    {
        std::shared_ptr<AudioBuffer> resampled_buffer;
        audio_resampler_->ResampleFrame(frame, resampled_buffer);
        audio_renderer_queue_.push(resampled_buffer);

        rendering_event.type = SDL_EVENTS[AUDIO_RENDERING];
        SDL_PushEvent(&rendering_event);
    }
}

void VideoPlayer::renderingVideo()
{
    std::lock_guard<std::mutex> lock(video_renderer_mutex_);

    std::shared_ptr<AVFrame> frame = video_renderer_queue_.front();
    if (frame)
    {
        video_renderer_->EnqueueFrame(frame);
    }
}

void VideoPlayer::renderingAudio()
{
    std::lock_guard<std::mutex> lock(audio_renderer_mutex_);

    std::shared_ptr<AudioBuffer> buffer = audio_renderer_queue_.front();
    if (buffer)
    {
        audio_renderer_->EnqueueAudioBuffer(buffer);
    }
}

void VideoPlayer::flushMediaQueue()
{
    std::lock_guard<std::mutex> lock(mutex_);

    video_decoder_queue_.flush();
    audio_decoder_queue_.flush();
    video_converter_queue_.flush();
    audio_resampler_queue_.flush();
    video_renderer_queue_.flush();
    audio_renderer_queue_.flush();
}
