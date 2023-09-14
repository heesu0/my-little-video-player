#include "../src/audio_buffer.h"
#include "../src/audio_resampler.h"
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

        auto audio_decoder = std::make_shared<Decoder>(demuxer->format_context(), demuxer->audio_stream_index());
        audio_decoder->Init();

        auto audio_resampler = std::make_shared<AudioResampler>(audio_decoder->codec_context());
        audio_resampler->Init();

        while (true)
        {
            std::shared_ptr<AVPacket> packet;
            if (demuxer->GetPacket(packet))
            {
                if (packet->stream_index == audio_decoder->stream_index())
                {
                    std::queue<std::shared_ptr<AVFrame>> frame_queue;
                    audio_decoder->GetFrame(packet, frame_queue);
                    while (!frame_queue.empty())
                    {
                        std::shared_ptr<AudioBuffer> audio_buffer;
                        std::shared_ptr<AVFrame> frame = frame_queue.front();
                        audio_resampler->ResampleFrame(frame, audio_buffer);
                        frame_queue.pop();
                        std::cout << "Resample Audio Frame : " << audio_buffer->size() << std::endl;
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
