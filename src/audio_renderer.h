#ifndef __AUDIO_RENDERER_H__
#define __AUDIO_RENDERER_H__

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
}

#include <SDL.h>
#include <memory>
#include <mutex>
#include <queue>

class AudioBuffer;

class AudioRenderer
{
public:
    explicit AudioRenderer(const std::shared_ptr<AVCodecContext>& codec_context, AVRational time_base);
    ~AudioRenderer();

    void Init();
    void Start() const;
    void Stop() const;
    void Flush();
    void EnqueueAudioBuffer(const std::shared_ptr<AudioBuffer>& audio_buffer);
    int GetAudioBuffer(uint8_t* audio_buffer, int buffer_size);
    uint64_t GetAudioTime() const;

private:
    std::shared_ptr<AVCodecContext> codec_context_;
    std::queue<std::shared_ptr<AudioBuffer>> buffer_queue_;

    std::mutex mutex_;

    AVRational time_base_;
    uint64_t audio_clock_ = 0;
    SDL_AudioDeviceID device_id_ = 0;
};

#endif
