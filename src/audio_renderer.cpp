#include "audio_renderer.h"

#include "audio_buffer.h"
#include "log.h"
#include "timer.h"

#include <algorithm>
#include <iostream>
#include <utility>

namespace {

constexpr int AUDIO_BUFFER_SIZE = 1024;
constexpr int MAX_AUDIO_FRAME_SIZE = 192000;
uint64_t audio_time = 0;

void AUDIO_CALLBACK(void* userdata, Uint8* stream, int len)
{
    auto* audio_renderer = reinterpret_cast<AudioRenderer*>(userdata);
    Timer::GetInstance()->SetAudioTime(audio_time);

    static uint8_t audio_buffer[(MAX_AUDIO_FRAME_SIZE * 3) / 2];
    static unsigned int audio_buffer_size = 0;
    static unsigned int audio_buffer_index = 0;

    while (len > 0)
    {
        if (audio_buffer_index >= audio_buffer_size)
        {
            int buffer_size = audio_renderer->GetAudioBuffer(audio_buffer, sizeof(audio_buffer));
            if (buffer_size < 0)
            {
                audio_buffer_size = AUDIO_BUFFER_SIZE;
                // Clear audio buffer
                std::fill(std::begin(audio_buffer), std::end(audio_buffer), 0);
            }
            else
            {
                audio_buffer_size = buffer_size;
            }

            audio_buffer_index = 0;
        }

        int len_to_copy = static_cast<int>(audio_buffer_size - audio_buffer_index);
        if (len_to_copy > len)
        {
            len_to_copy = len;
        }

        std::fill(stream, stream + len_to_copy, 0);
        SDL_MixAudioFormat(stream, audio_buffer + audio_buffer_index, AUDIO_S16SYS, len_to_copy, 100);

        len -= len_to_copy;
        stream += len_to_copy;
        audio_buffer_index += len_to_copy;
    }

    audio_time = audio_renderer->GetAudioTime();
}

} // namespace

AudioRenderer::AudioRenderer(const std::shared_ptr<AVCodecContext>& codec_context, AVRational time_base)
    : codec_context_(codec_context), time_base_(time_base)
{
}

AudioRenderer::~AudioRenderer() = default;

void AudioRenderer::Init()
{
    if (SDL_Init(SDL_INIT_AUDIO))
    {
        LOG::ERROR_FROM_SDL();
    }

    SDL_AudioSpec spec;
    SDL_AudioSpec wanted_spec;
    SDL_memset(&wanted_spec, 0, sizeof(wanted_spec));
    wanted_spec.freq = codec_context_->sample_rate;
    wanted_spec.format = AUDIO_S16SYS;
    wanted_spec.channels = codec_context_->channels;
    wanted_spec.silence = 0;
    wanted_spec.samples = AUDIO_BUFFER_SIZE;
    wanted_spec.callback = AUDIO_CALLBACK;
    wanted_spec.userdata = this;

    device_id_ = SDL_OpenAudioDevice(nullptr, 0, &wanted_spec, &spec, 0);
    if (device_id_ == 0)
    {
        LOG::ERROR_FROM_SDL();
    }

    SDL_PauseAudioDevice(device_id_, 0);
}

void AudioRenderer::Start() const
{
    SDL_PauseAudioDevice(device_id_, 0);
}

void AudioRenderer::Stop() const
{
    SDL_PauseAudioDevice(device_id_, 1);
}

void AudioRenderer::Flush()
{
    Stop();
    {
        std::lock_guard<std::mutex> lock(mutex_);
        buffer_queue_ = std::queue<std::shared_ptr<AudioBuffer>>();
        SDL_ClearQueuedAudio(device_id_);
    }
    Start();
}

void AudioRenderer::EnqueueAudioBuffer(const std::shared_ptr<AudioBuffer>& audio_buffer)
{
    std::lock_guard<std::mutex> lock(mutex_);
    buffer_queue_.push(audio_buffer);
}

int AudioRenderer::GetAudioBuffer(uint8_t* audio_buffer, int buffer_size)
{
    if (!buffer_queue_.empty())
    {
        std::lock_guard<std::mutex> lock(mutex_);

        auto front_buffer = buffer_queue_.front();
        if (buffer_size > static_cast<int>(front_buffer->size()))
        {
            buffer_size = static_cast<int>(front_buffer->size());
        }

        std::copy(front_buffer->buffer(), front_buffer->buffer() + buffer_size, audio_buffer);
        audio_clock_ = static_cast<uint64_t>(av_q2d(time_base_) * front_buffer->pts() * 1000);
        buffer_queue_.pop();

        return buffer_size;
    }

    return -1;
}

uint64_t AudioRenderer::GetAudioTime() const
{
    return audio_clock_;
}
