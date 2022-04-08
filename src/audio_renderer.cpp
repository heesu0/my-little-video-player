#include "audio_renderer.h"
#include "log.h"
#include "timer.h"
#include <algorithm>
#include <utility>

namespace {

constexpr int SDL_AUDIO_BUFFER_SIZE = 1024;
constexpr int MAX_AUDIO_FRAME_SIZE = 192000;
uint64_t audio_time = 0;

void AUDIO_CALLBACK(void* userdata, Uint8* stream, int len) {
  Timer::getInstance()->setAudioTime(audio_time);
  auto* audio_renderer = reinterpret_cast<AudioRenderer*>(userdata);

  static uint8_t audio_buffer[(MAX_AUDIO_FRAME_SIZE * 3) / 2];
  static unsigned int audio_buffer_size = 0;
  static unsigned int audio_buffer_index = 0;
  int audio_bytes_len = len;
  int len1 = -1;
  int audio_size = -1;


  while (len > 0) {
    if (audio_buffer_index >= audio_buffer_size) {
      audio_size = audio_renderer->getAudioBuffer(audio_buffer,
                                                  sizeof(audio_buffer));
      if (audio_size < 0) {
        audio_buffer_size = 1024;
        // Clear audio buffer
        std::fill(audio_buffer, audio_buffer + (MAX_AUDIO_FRAME_SIZE * 3) / 2,
                  0);
      } else {
        audio_buffer_size = audio_size;
      }
      audio_buffer_index = 0;
    }
    len1 = static_cast<int>(audio_buffer_size - audio_buffer_index);
    if (len1 > len) {
      len1 = len;
    }

    std::fill(stream, stream + len1, 0);
    SDL_MixAudioFormat(stream, audio_buffer + audio_buffer_index, AUDIO_S16SYS,
                       len1, 100);
    len -= len1;
    stream += len1;
    audio_buffer_index += len1;
  }

  audio_time = audio_renderer->getAudioTime() -
               audio_renderer->getMsFromBytes(audio_bytes_len) -
               2 * audio_renderer->getMsFromBytes(audio_buffer_size);
}

}//namespace

AudioRenderer::AudioRenderer(std::shared_ptr<AVCodecContext>& codec_context,
                             AVRational time_base)
    : codec_context_(codec_context), time_base_(time_base), audio_clock_(0),
      bytes_per_sec_(0), device_id_(0) {}

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
  bytes_per_sec_ = av_samples_get_buffer_size(
          nullptr, wanted_spec.channels, wanted_spec.freq,
          static_cast<AVSampleFormat>(wanted_spec.format), 1);

  device_id_ = SDL_OpenAudioDevice(nullptr, 0, &wanted_spec, &spec, 0);
  if (device_id_ == 0) {
    LOG::ERROR_FROM_SDL();
  }

  SDL_PauseAudioDevice(device_id_, 0);
}

void AudioRenderer::start() const { SDL_PauseAudioDevice(device_id_, 0); }

void AudioRenderer::stop() const { SDL_PauseAudioDevice(device_id_, 1); }

void AudioRenderer::flush() {
  stop();
  mutex_.lock();
  buffer_queue_ = std::queue<std::shared_ptr<AudioBuffer>>();
  SDL_ClearQueuedAudio(device_id_);
  mutex_.unlock();
  start();
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
    if (buffer_size > static_cast<int>(front_buffer->size())) {
      buffer_size = static_cast<int>(front_buffer->size());
    }
    std::copy(front_buffer->buffer(), front_buffer->buffer() + buffer_size,
              audio_buffer);
    buffer_queue_.pop();
    audio_clock_ = av_q2d(time_base_) * front_buffer->pts() * 1000;
    mutex_.unlock();
    return buffer_size;
  }
  return -1;
}

uint64_t AudioRenderer::getMsFromBytes(uint32_t len) const {
  return (len * 1000) / bytes_per_sec_;
}
