#include "audio_renderer.h"
#include "log.h"
#include <iostream>
#include <utility>

namespace {

constexpr int SDL_AUDIO_BUFFER_SIZE = 1024;
constexpr int MAX_AUDIO_FRAME_SIZE = 192000;

void AUDIO_CALLBACK(void* userdata, Uint8* stream, int len) {
  auto* audio_renderer = reinterpret_cast<AudioRenderer*>(userdata);

  static uint8_t audio_buffer[(MAX_AUDIO_FRAME_SIZE * 3) / 2];
  static unsigned int audio_buffer_size = 0;
  static unsigned int audio_buffer_index = 0;
  int len1 = -1;
  int audio_size = -1;

  while (len > 0) {
    if (audio_buffer_index >= audio_buffer_size) {
      audio_size = audio_renderer->getAudioBuffer(audio_buffer,
                                                  sizeof(audio_buffer));
      if (audio_size < 0) {
        audio_buffer_size = 1024;
        // Clear audio buffer
        memset(audio_buffer, 0, (MAX_AUDIO_FRAME_SIZE * 3) / 2);
      } else {
        audio_buffer_size = audio_size;
      }
      audio_buffer_index = 0;
    }
    len1 = static_cast<int>(audio_buffer_size - audio_buffer_index);
    if (len1 > len) {
      len1 = len;
    }

    memset(stream, 0, len1);
    SDL_MixAudioFormat(stream,
                       reinterpret_cast<uint8_t*>(audio_buffer) +
                               audio_buffer_index,
                       AUDIO_S16SYS, len1, 100);
    len -= len1;
    stream += len1;
    audio_buffer_index += len1;
  }
}

}//namespace

AudioRenderer::AudioRenderer(std::shared_ptr<AVCodecContext>& codec_context)
    : codec_context_(codec_context) {}

AudioRenderer::~AudioRenderer() = default;

void AudioRenderer::init() {
  if (SDL_Init(SDL_INIT_AUDIO)) {
    LOG::ERROR_FROM_SDL();
  }

  SDL_AudioSpec spec;
  SDL_AudioSpec wanted_spec;
  SDL_memset(&wanted_spec, 0, sizeof(wanted_spec));
  wanted_spec.freq = codec_context_->sample_rate;
  wanted_spec.format = AUDIO_S16SYS;
  wanted_spec.channels = codec_context_->channels;
  wanted_spec.silence = 0;
  wanted_spec.samples = SDL_AUDIO_BUFFER_SIZE;
  wanted_spec.callback = AUDIO_CALLBACK;
  wanted_spec.userdata = this;

  SDL_AudioDeviceID device_id =
          SDL_OpenAudioDevice(nullptr, 0, &wanted_spec, &spec, 0);
  if (device_id == 0) {
    LOG::ERROR_FROM_SDL();
  }

  SDL_PauseAudioDevice(device_id, 0);
}

void AudioRenderer::enqueueAudioBuffer(
        std::shared_ptr<AudioBuffer>& audio_buffer) {
  mutex_.lock();
  buffer_queue_.push(audio_buffer);
  mutex_.unlock();
}

int AudioRenderer::getAudioBuffer(uint8_t* audio_buffer, int buffer_size) {
  if (!buffer_queue_.empty()) {
    mutex_.lock();
    auto front_buffer = buffer_queue_.front();
    buffer_size = buffer_size > front_buffer->size() ? front_buffer->size()
                                                     : buffer_size;
    memcpy(audio_buffer, front_buffer->buffer(), buffer_size);
    buffer_queue_.pop();
    mutex_.unlock();
    return buffer_size;
  }
  return -1;
}
