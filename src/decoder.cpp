#include "decoder.h"

#include "log.h"

#include <iostream>

Decoder::Decoder(const std::shared_ptr<AVFormatContext>& format_context, int stream_index)
    : format_context_(format_context), stream_index_(stream_index)
{
}

void Decoder::Init()
{
    const auto* codec = avcodec_find_decoder(format_context_->streams[stream_index_]->codecpar->codec_id);
    if (!codec)
    {
        LOG::ERROR("Couldn't find AVCodec");
    }

    codec_context_ = std::shared_ptr<AVCodecContext>(avcodec_alloc_context3(codec), [](AVCodecContext* codec_context) {
        avcodec_close(codec_context);
        avcodec_free_context(&codec_context);
        std::cout << "AVCodec close" << std::endl;
    });
    if (!codec_context_)
    {
        LOG::ERROR("Couldn't allocate AVCodecContext");
    }

    int ret = avcodec_parameters_to_context(codec_context_.get(), format_context_->streams[stream_index_]->codecpar);
    if (ret < 0)
    {
        LOG::ERROR_FROM_FFMPEG(ret);
    }

    ret = avcodec_open2(codec_context_.get(), codec, nullptr);
    if (ret < 0)
    {
        LOG::ERROR_FROM_FFMPEG(ret);
    }
}

void Decoder::Flush() const
{
    avcodec_flush_buffers(codec_context_.get());
}

bool Decoder::GetFrame(const std::shared_ptr<AVPacket>& packet, std::queue<std::shared_ptr<AVFrame>>& frame_queue) const
{
    int ret = avcodec_send_packet(codec_context_.get(), packet.get());
    if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
    {
        return false;
    }

    if (ret < 0)
    {
        LOG::ERROR_FROM_FFMPEG(ret);
    }

    while (true)
    {
        AVFrame* frame = av_frame_alloc();
        ret = avcodec_receive_frame(codec_context_.get(), frame);
        if (ret >= 0)
        {
            frame_queue.push(std::shared_ptr<AVFrame>(frame, [](AVFrame* frame) { av_frame_free(&frame); }));
        }
        else
        {
            av_frame_free(&frame);
            break;
        }
    }

    return true;
}
