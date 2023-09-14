#ifndef __VIDEO_CONVERTER_H__
#define __VIDEO_CONVERTER_H__

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/imgutils.h>
#include <libswscale/swscale.h>
}

#include <memory>

class VideoConverter
{
public:
    explicit VideoConverter(const std::shared_ptr<AVCodecContext>& codec_context);
    ~VideoConverter() = default;

    void Init();
    void ConvertFrame(const std::shared_ptr<AVFrame>& input_frame, std::shared_ptr<AVFrame>& output_frame) const;

private:
    std::shared_ptr<AVCodecContext> codec_context_;
    std::shared_ptr<SwsContext> sws_context_;
};

#endif
