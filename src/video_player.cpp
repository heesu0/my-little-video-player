#ifdef __linux__
#include <SDL2/SDL.h>
#elif __APPLE__
#include <SDL.h>
#endif
#include "audio_buffer.h"
#include "audio_renderer.h"
#include "audio_resampler.h"
#include "decoder.h"
#include "demuxer.h"
#include "task_queue.h"
#include "timer.h"
#include "video_converter.h"
#include "video_renderer.h"
#include <atomic>
#include <chrono>
#include <future>
#include <iostream>
#include <memory>
#include <mutex>
#include <vector>

constexpr int MAX_COUNT = 20;

std::atomic<int> demuxing_count(0);

TaskQueue<AVPacket> video_decoder_queue;
TaskQueue<AVPacket> audio_decoder_queue;
TaskQueue<AVFrame> video_converter_queue;
TaskQueue<AVFrame> audio_resampler_queue;
TaskQueue<AVFrame> video_renderer_queue;
TaskQueue<AudioBuffer> audio_renderer_queue;

std::mutex demuxer_lock, video_decoder_lock, audio_decoder_lock,
        video_converter_lock, audio_resampler_lock, video_renderer_lock,
        audio_renderer_lock;

Uint32 SDL_DEMUXING, SDL_VIDEO_DECODING, SDL_AUDIO_DECODING,
        SDL_VIDEO_CONVERTING, SDL_AUDIO_RESAMPLING, SDL_VIDEO_RENDERING,
        SDL_AUDIO_RENDERING, SDL_DEMUXING_COMPLETED;

void demuxing(const std::shared_ptr<Demuxer>& demuxer);
void videoDecoding(const std::shared_ptr<Decoder>& decoder);
void audioDecoding(const std::shared_ptr<Decoder>& decoder);
void videoConverting(const std::shared_ptr<VideoConverter>& converter);
void audioResampling(const std::shared_ptr<AudioResampler>& resampler);
void videoRendering(const std::shared_ptr<VideoRenderer>& renderer);
void audioRendering(const std::shared_ptr<AudioRenderer>& renderer);
void flushTaskQueue();

int main(int argc, char** argv) {
  try {
    if (argc != 2) {
      throw std::logic_error("Invalid arguments");
    }

    auto demuxer = std::make_shared<Demuxer>(argv[1]);
    demuxer->init();

    auto video_decoder = std::make_shared<Decoder>(
            demuxer->video_stream_index(), demuxer->format_context());
    video_decoder->init();

    auto audio_decoder = std::make_shared<Decoder>(
            demuxer->audio_stream_index(), demuxer->format_context());
    audio_decoder->init();

    auto video_codec_context = video_decoder->codec_context();
    auto audio_codec_context = audio_decoder->codec_context();

    auto video_converter =
            std::make_shared<VideoConverter>(video_codec_context);
    video_converter->init();

    auto audio_resampler =
            std::make_shared<AudioResampler>(audio_codec_context);
    audio_resampler->init();

    auto video_time_base = demuxer->format_context()
                                   ->streams[video_decoder->stream_index()]
                                   ->time_base;
    auto audio_time_base = demuxer->format_context()
                                   ->streams[audio_decoder->stream_index()]
                                   ->time_base;

    auto video_renderer = std::make_shared<VideoRenderer>(video_codec_context,
                                                          video_time_base);
    video_renderer->init();

    auto audio_renderer = std::make_shared<AudioRenderer>(audio_codec_context,
                                                          audio_time_base);
    audio_renderer->init();

    std::vector<std::future<void>> task_queue;
    bool demuxing_completed = false;
    bool running = true;

    SDL_DEMUXING = SDL_RegisterEvents(1);
    SDL_VIDEO_DECODING = SDL_RegisterEvents(2);
    SDL_AUDIO_DECODING = SDL_RegisterEvents(3);
    SDL_VIDEO_CONVERTING = SDL_RegisterEvents(4);
    SDL_AUDIO_RESAMPLING = SDL_RegisterEvents(5);
    SDL_VIDEO_RENDERING = SDL_RegisterEvents(6);
    SDL_AUDIO_RENDERING = SDL_RegisterEvents(7);
    SDL_DEMUXING_COMPLETED = SDL_RegisterEvents(8);
    SDL_Event event;
    while (true) {
      SDL_PollEvent(&event);

      if (event.type == SDL_QUIT) {
        flushTaskQueue();
        std::cout << "Press Quit" << std::endl;
        SDL_Quit();
        break;
      } else if (event.type == SDL_KEYDOWN) {
        switch (event.key.keysym.sym) {
          case SDLK_p:
          case SDLK_SPACE:
            std::cout << "Press Space" << std::endl;
            running = !running;
            if (running) {
              video_renderer->start();
              audio_renderer->start();
            } else {
              video_renderer->stop();
              audio_renderer->stop();
            }
            break;
          case SDLK_LEFT:
            std::cout << "Press Reverse" << std::endl;
            flushTaskQueue();
            for (auto& task : task_queue) {
              task.get();
            }
            task_queue.clear();
            video_renderer->flush();
            audio_renderer->flush();
            video_decoder->flush();
            audio_decoder->flush();
            SDL_PumpEvents();
            demuxing_count = 0;
            Timer::getInstance()->setAudioTime(
                    Timer::getInstance()->getAudioTime() - (10 * 1000));
            demuxer->seek(Timer::getInstance()->getAudioTime());
            demuxing_completed = false;
            break;
          case SDLK_RIGHT:
            std::cout << "Press Forward" << std::endl;
            flushTaskQueue();
            for (auto& task : task_queue) {
              task.get();
            }
            task_queue.clear();
            video_renderer->flush();
            audio_renderer->flush();
            video_decoder->flush();
            audio_decoder->flush();
            SDL_PumpEvents();
            demuxing_count = 0;
            Timer::getInstance()->setAudioTime(
                    Timer::getInstance()->getAudioTime() + (10 * 1000));
            demuxer->seek(Timer::getInstance()->getAudioTime());
            demuxing_completed = false;
            break;
        }
      } else if (event.type == SDL_DEMUXING) {
        task_queue.push_back(std::async(std::launch::async, demuxing, demuxer));
      } else if (event.type == SDL_VIDEO_DECODING) {
        task_queue.push_back(
                std::async(std::launch::async, videoDecoding, video_decoder));
      } else if (event.type == SDL_AUDIO_DECODING) {
        task_queue.push_back(
                std::async(std::launch::async, audioDecoding, audio_decoder));
      } else if (event.type == SDL_VIDEO_CONVERTING) {
        task_queue.push_back(std::async(std::launch::async, videoConverting,
                                        video_converter));
      } else if (event.type == SDL_AUDIO_RESAMPLING) {
        task_queue.push_back(std::async(std::launch::async, audioResampling,
                                        audio_resampler));
      } else if (event.type == SDL_VIDEO_RENDERING) {
        task_queue.push_back(
                std::async(std::launch::async, videoRendering, video_renderer));
      } else if (event.type == SDL_AUDIO_RENDERING) {
        task_queue.push_back(
                std::async(std::launch::async, audioRendering, audio_renderer));
      } else if (event.type == SDL_DEMUXING_COMPLETED) {
        demuxing_completed = true;
      }

      std::vector<int> task_completed;

      for (int i = 0; i < static_cast<int>(task_queue.size()); i++) {
        auto status = task_queue[i].wait_for(std::chrono::milliseconds(10));
        if (status == std::future_status::ready) {
          task_completed.push_back(i);
        }
      }

      for (int j = static_cast<int>(task_completed.size()) - 1; j >= 0; j--) {
        task_queue.erase(task_queue.begin() + task_completed[j]);
      }

      if (!demuxing_completed && running) {
        if (video_renderer_queue.size() < MAX_COUNT ||
            audio_renderer_queue.size() < MAX_COUNT) {
          if (demuxing_count < 20) {
            SDL_Event demuxing_event;
            demuxing_event.type = SDL_DEMUXING;
            demuxing_count++;
            SDL_PushEvent(&demuxing_event);
          }
        }
      }
    }

    for (auto& task : task_queue) {
      task.get();
    }

  } catch (const std::exception& e) {
    std::cerr << e.what() << std::endl;
    return -1;
  }

  return 0;
}

void demuxing(const std::shared_ptr<Demuxer>& demuxer) {
  std::lock_guard<std::mutex> lock_guard(demuxer_lock);
  std::shared_ptr<AVPacket> packet;
  SDL_Event demuxing_event;
  if (demuxer->getPacket(packet)) {
    if (packet->stream_index == demuxer->video_stream_index()) {
      video_decoder_queue.push(packet);
      demuxing_event.type = SDL_VIDEO_DECODING;
    } else if (packet->stream_index == demuxer->audio_stream_index()) {
      audio_decoder_queue.push(packet);
      demuxing_event.type = SDL_AUDIO_DECODING;
    }
  } else {
    demuxing_event.type = SDL_DEMUXING_COMPLETED;
  }

  SDL_PushEvent(&demuxing_event);
  demuxing_count--;
}

void videoDecoding(const std::shared_ptr<Decoder>& decoder) {
  std::lock_guard<std::mutex> lock_guard(video_decoder_lock);
  std::shared_ptr<AVPacket> packet = video_decoder_queue.front();
  if (packet) {
    std::queue<std::shared_ptr<AVFrame>> frame_queue;
    decoder->getFrame(packet, frame_queue);
    while (!frame_queue.empty()) {
      std::shared_ptr<AVFrame> frame = frame_queue.front();
      video_converter_queue.push(frame);
      frame_queue.pop();
      SDL_Event converting_event;
      converting_event.type = SDL_VIDEO_CONVERTING;
      SDL_PushEvent(&converting_event);
    }
  }
}

void audioDecoding(const std::shared_ptr<Decoder>& decoder) {
  std::lock_guard<std::mutex> lock_guard(audio_decoder_lock);
  std::shared_ptr<AVPacket> packet = audio_decoder_queue.front();
  if (packet) {
    std::queue<std::shared_ptr<AVFrame>> frame_queue;
    decoder->getFrame(packet, frame_queue);
    while (!frame_queue.empty()) {
      std::shared_ptr<AVFrame> frame = frame_queue.front();
      audio_resampler_queue.push(frame);
      frame_queue.pop();
      SDL_Event resampling_event;
      resampling_event.type = SDL_AUDIO_RESAMPLING;
      SDL_PushEvent(&resampling_event);
    }
  }
}

void videoConverting(const std::shared_ptr<VideoConverter>& converter) {
  std::lock_guard<std::mutex> lock_guard(video_converter_lock);
  std::shared_ptr<AVFrame> frame = video_converter_queue.front();
  if (frame) {
    std::shared_ptr<AVFrame> converted_frame;
    converter->convertFrame(frame, converted_frame);
    video_renderer_queue.push(converted_frame);
    SDL_Event rendering_event;
    rendering_event.type = SDL_VIDEO_RENDERING;
    SDL_PushEvent(&rendering_event);
  }
}

void audioResampling(const std::shared_ptr<AudioResampler>& resampler) {
  std::lock_guard<std::mutex> lock_guard(audio_resampler_lock);
  std::shared_ptr<AVFrame> frame = audio_resampler_queue.front();
  if (frame) {
    std::shared_ptr<AudioBuffer> resampled_buffer;
    resampler->resampleFrame(frame, resampled_buffer);
    audio_renderer_queue.push(resampled_buffer);
    SDL_Event rendering_event;
    rendering_event.type = SDL_AUDIO_RENDERING;
    SDL_PushEvent(&rendering_event);
  }
}

void videoRendering(const std::shared_ptr<VideoRenderer>& renderer) {
  std::lock_guard<std::mutex> lock_guard(video_renderer_lock);
  std::shared_ptr<AVFrame> frame = video_renderer_queue.front();
  if (frame) {
    renderer->enqueueFrame(frame);
  }
}

void audioRendering(const std::shared_ptr<AudioRenderer>& renderer) {
  std::lock_guard<std::mutex> lock_guard(audio_renderer_lock);
  std::shared_ptr<AudioBuffer> buffer = audio_renderer_queue.front();
  if (buffer) {
    renderer->enqueueAudioBuffer(buffer);
  }
}

void flushTaskQueue() {
  video_decoder_queue.flush();
  audio_decoder_queue.flush();
  video_converter_queue.flush();
  audio_resampler_queue.flush();
  video_renderer_queue.flush();
  audio_renderer_queue.flush();
}
