#include "../src/audio_buffer.h"
#include "../src/audio_resampler.h"
#include "../src/decoder.h"
#include "../src/demuxer.h"
#include "../src/video_converter.h"
#include <iostream>
#include <memory>
#include <queue>

int main(int argc, char** argv) {
  try {
    if (argc != 2) {
      throw std::logic_error("Invalid arguments");
    }

    std::unique_ptr<Demuxer> demuxer = std::make_unique<Demuxer>(argv[1]);
    demuxer->init();

    int video_stream_index = demuxer->video_stream_index();
    int audio_stream_index = demuxer->audio_stream_index();
    auto format_context = demuxer->format_context();

    auto video_decoder =
            std::make_shared<Decoder>(video_stream_index, format_context);
    auto audio_decoder =
            std::make_shared<Decoder>(audio_stream_index, format_context);

    video_decoder->init();
    audio_decoder->init();

    auto video_codec_context = video_decoder->codec_context();
    auto video_converter =
            std::make_shared<VideoConverter>(video_codec_context);
    video_converter->init();

    auto audio_codec_context = audio_decoder->codec_context();
    auto audio_resampler =
            std::make_shared<AudioResampler>(audio_codec_context);
    audio_resampler->init();

    while (true) {
      std::shared_ptr<AVPacket> packet;
      if (demuxer->getPacket(packet)) {
        if (packet->stream_index == video_decoder->stream_index()) {
          std::queue<std::shared_ptr<AVFrame>> frame_queue;
          video_decoder->getFrame(packet, frame_queue);
          while (!frame_queue.empty()) {
            std::shared_ptr<AVFrame> converted_frame;
            std::shared_ptr<AVFrame> frame = frame_queue.front();
            video_converter->convertFrame(frame, converted_frame);
            frame_queue.pop();
            std::cout << "Convert Video Frame" << std::endl;
          }
        } else if (packet->stream_index == audio_decoder->stream_index()) {
          std::queue<std::shared_ptr<AVFrame>> frame_queue;
          audio_decoder->getFrame(packet, frame_queue);
          while (!frame_queue.empty()) {
            std::shared_ptr<AudioBuffer> audio_buffer;
            std::shared_ptr<AVFrame> frame = frame_queue.front();
            audio_resampler->resampleFrame(frame, audio_buffer);
            frame_queue.pop();
            std::cout << "Resample Audio Frame : " << audio_buffer->size
                      << std::endl;
          }
        }
      } else {
        break;
      }
    }
  } catch (const std::exception& e) {
    std::cerr << e.what() << std::endl;
    return -1;
  }

  return 0;
}
