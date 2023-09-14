#include "../src/decoder.h"
#include "../src/demuxer.h"

#include <iostream>
#include <memory>
#include <queue>

int main(int argc, char** argv)
{
    try
    {
        if (argc != 2)
        {
            throw std::logic_error("Invalid arguments");
        }

        auto demuxer = std::make_unique<Demuxer>(argv[1]);
        demuxer->Init();

        int video_stream_index = demuxer->video_stream_index();
        int audio_stream_index = demuxer->audio_stream_index();
        auto format_context = demuxer->format_context();

        auto video_decoder = std::make_shared<Decoder>(format_context, video_stream_index);
        auto audio_decoder = std::make_shared<Decoder>(format_context, audio_stream_index);

        video_decoder->Init();
        audio_decoder->Init();

        while (true)
        {
            std::shared_ptr<AVPacket> packet;
            if (demuxer->GetPacket(packet))
            {
                std::queue<std::shared_ptr<AVFrame>> frame_queue;
                if (packet->stream_index == video_decoder->stream_index())
                {
                    video_decoder->GetFrame(packet, frame_queue);
                    while (!frame_queue.empty())
                    {
                        frame_queue.pop();
                        std::cout << "Video frame" << std::endl;
                    }
                }
                else if (packet->stream_index == audio_decoder->stream_index())
                {
                    audio_decoder->GetFrame(packet, frame_queue);
                    while (!frame_queue.empty())
                    {
                        frame_queue.pop();
                        std::cout << "Audio frame" << std::endl;
                    }
                }
            }
            else
            {
                break;
            }
        }
    }
    catch (const std::exception& e)
    {
        std::cerr << e.what() << std::endl;
        return -1;
    }

    return 0;
}
