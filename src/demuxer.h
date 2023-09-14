#ifndef __DEMUXER_H__
#define __DEMUXER_H__

extern "C" {
#include <libavformat/avformat.h>
}

#include <memory>
#include <string>

class Demuxer
{
public:
    explicit Demuxer(const std::string& filename);
    ~Demuxer() = default;

    void Init();
    void Seek(uint64_t time) const;
    bool GetPacket(std::shared_ptr<AVPacket>& packet) const;
    void PrintFileInfo() const;

    std::shared_ptr<AVFormatContext>& format_context()
    {
        return format_context_;
    }
    int video_stream_index() const
    {
        return video_stream_index_;
    }
    int audio_stream_index() const
    {
        return audio_stream_index_;
    }

private:
    std::shared_ptr<AVFormatContext> format_context_;

    std::string filename_;

    int video_stream_index_ = -1;
    int audio_stream_index_ = -1;
};

#endif
