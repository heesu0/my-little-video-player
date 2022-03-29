#include "audio_buffer.h"
#include <algorithm>

AudioBuffer::AudioBuffer(int size) : buffer_(new uint8_t[size]), size_(size) {}

AudioBuffer::~AudioBuffer() { delete[] buffer_; }

AudioBuffer::AudioBuffer(const AudioBuffer& audio_buffer)
    : buffer_(new uint8_t[audio_buffer.size_]), size_(audio_buffer.size_) {
  std::copy(audio_buffer.buffer_, audio_buffer.buffer_ + size_, buffer_);
}

AudioBuffer& AudioBuffer::operator=(const AudioBuffer& audio_buffer) {
  if (this != &audio_buffer) {
    delete[] buffer_;

    size_ = audio_buffer.size_;
    buffer_ = new uint8_t[size_];
    std::copy(audio_buffer.buffer_, audio_buffer.buffer_ + size_, buffer_);
  }
  return *this;
}

AudioBuffer::AudioBuffer(AudioBuffer&& audio_buffer) noexcept
    : buffer_(nullptr), size_(0) {
  buffer_ = audio_buffer.buffer_;
  size_ = audio_buffer.size_;

  audio_buffer.buffer_ = nullptr;
  audio_buffer.size_ = 0;
}


AudioBuffer& AudioBuffer::operator=(AudioBuffer&& audio_buffer) noexcept {
  if (this != &audio_buffer) {
    delete[] buffer_;
    buffer_ = nullptr;

    buffer_ = audio_buffer.buffer_;
    size_ = audio_buffer.size_;

    audio_buffer.buffer_ = nullptr;
    audio_buffer.size_ = 0;
  }
  return *this;
}

uint8_t* AudioBuffer::buffer() const { return buffer_; }

int AudioBuffer::size() const { return size_; }
