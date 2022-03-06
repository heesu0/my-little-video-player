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

    int video_stream_index = demuxer->video_stream_index();
    auto format_context = demuxer->format_context();

    auto video_decoder =
            std::make_shared<Decoder>(video_stream_index, format_context);

    video_decoder->init();

    auto video_codec_context = video_decoder->codec_context();
    auto video_converter =
            std::make_shared<VideoConverter>(video_codec_context);
    video_converter->init();

    auto video_renderer = std::make_shared<VideoRenderer>(video_codec_context);
    video_renderer->init();

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
            double fps = av_q2d(
                    format_context->streams[video_stream_index]->r_frame_rate);
            double delay = 1 / static_cast<double>(fps);
            SDL_Delay(static_cast<uint32_t>(1000 * delay) - 10);

            std::shared_ptr<AVFrame> converted_frame;
            std::shared_ptr<AVFrame> frame = frame_queue.front();
            video_converter->convertFrame(frame, converted_frame);
            video_renderer->renderFrame(converted_frame);
            frame_queue.pop();
            std::cout << "Video frame" << std::endl;
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
