#ifndef __DECODER_H__
#define __DECODER_H__

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
}

#include <memory>
#include <queue>
#include <string>

class Decoder
{
public:
    explicit Decoder(const std::shared_ptr<AVFormatContext>& format_context, int stream_index);
    ~Decoder() = default;

    void Init();
    void Flush() const;
    bool GetFrame(const std::shared_ptr<AVPacket>& packet, std::queue<std::shared_ptr<AVFrame>>& frame_queue) const;

    int stream_index() const
    {
        return stream_index_;
    }
    std::shared_ptr<AVCodecContext>& codec_context()
    {
        return codec_context_;
    }

private:
    std::shared_ptr<AVFormatContext> format_context_;
    std::shared_ptr<AVCodecContext> codec_context_;

    int stream_index_;
};

#endif
