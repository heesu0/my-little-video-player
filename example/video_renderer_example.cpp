#include "../src/audio_buffer.h"
#include "../src/audio_renderer.h"
#include "../src/audio_resampler.h"
#include "../src/decoder.h"
#include "../src/demuxer.h"
#include "../src/video_converter.h"
#include "../src/video_renderer.h"
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

    auto video_decoder = std::make_shared<Decoder>(
            demuxer->video_stream_index(), demuxer->format_context());
    video_decoder->init();

    auto audio_decoder = std::make_shared<Decoder>(
            demuxer->audio_stream_index(), demuxer->format_context());
    audio_decoder->init();

    auto video_converter =
            std::make_shared<VideoConverter>(video_decoder->codec_context());
    video_converter->init();

    auto audio_resampler =
            std::make_shared<AudioResampler>(audio_decoder->codec_context());
    audio_resampler->init();

    auto video_time_base = demuxer->format_context()
                                   ->streams[video_decoder->stream_index()]
                                   ->time_base;
    auto video_renderer = std::make_shared<VideoRenderer>(
            video_decoder->codec_context(), video_time_base);
    video_renderer->init();

    auto audio_time_base = demuxer->format_context()
                                   ->streams[audio_decoder->stream_index()]
                                   ->time_base;
    auto audio_renderer = std::make_shared<AudioRenderer>(
            audio_decoder->codec_context(), audio_time_base);
    audio_renderer->init();

    SDL_Event event;
    while (true) {
      SDL_PollEvent(&event);
      if (event.type == SDL_QUIT) {
        std::cout << "SDL QUIT" << std::endl;
        SDL_Quit();
        break;
      }

      std::shared_ptr<AVPacket> packet;
      if (demuxer->getPacket(packet)) {
        if (packet->stream_index == video_decoder->stream_index()) {
          std::queue<std::shared_ptr<AVFrame>> frame_queue;
          video_decoder->getFrame(packet, frame_queue);
          while (!frame_queue.empty()) {
            std::shared_ptr<AVFrame> converted_frame;
            std::shared_ptr<AVFrame> frame = frame_queue.front();
            video_converter->convertFrame(frame, converted_frame);
            video_renderer->enqueueFrame(frame);
            frame_queue.pop();
          }
        } else if (packet->stream_index == audio_decoder->stream_index()) {
          std::queue<std::shared_ptr<AVFrame>> frame_queue;
          audio_decoder->getFrame(packet, frame_queue);
          while (!frame_queue.empty()) {
            std::shared_ptr<AudioBuffer> audio_buffer;
            std::shared_ptr<AVFrame> frame = frame_queue.front();
            audio_resampler->resampleFrame(frame, audio_buffer);
            audio_renderer->enqueueAudioBuffer(audio_buffer);
            frame_queue.pop();
          }
        }
      }
    }
  } catch (const std::exception& e) {
    std::cerr << e.what() << std::endl;
    return -1;
  }

  return 0;
}
