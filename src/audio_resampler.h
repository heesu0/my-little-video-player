#ifndef __AUDIO_RESAMPLER_H__
#define __AUDIO_RESAMPLER_H__

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswresample/swresample.h>
}

#include <memory>

class AudioBuffer;

class AudioResampler
{
public:
    explicit AudioResampler(const std::shared_ptr<AVCodecContext>& codec_context);
    ~AudioResampler() = default;

    void Init();
    void ResampleFrame(const std::shared_ptr<AVFrame>& input_frame, std::shared_ptr<AudioBuffer>& output_buffer);

private:
    std::shared_ptr<AVCodecContext> codec_context_;
    std::shared_ptr<SwrContext> swr_context_;
};

#endif
