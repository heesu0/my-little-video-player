#include "audio_buffer.h"
#include <algorithm>

AudioBuffer::AudioBuffer(uint64_t size, uint64_t pts)
    : buffer_(new uint8_t[size]), size_(size), pts_(pts) {}

AudioBuffer::~AudioBuffer() { delete[] buffer_; }

AudioBuffer::AudioBuffer(const AudioBuffer& audio_buffer)
    : buffer_(new uint8_t[audio_buffer.size_]), size_(audio_buffer.size_),
      pts_(audio_buffer.pts_) {
  std::copy(audio_buffer.buffer_, audio_buffer.buffer_ + size_, buffer_);
}

AudioBuffer& AudioBuffer::operator=(const AudioBuffer& audio_buffer) {
  if (this != &audio_buffer) {
    delete[] buffer_;

    pts_ = audio_buffer.pts_;
    size_ = audio_buffer.size_;
    buffer_ = new uint8_t[size_];
    std::copy(audio_buffer.buffer_, audio_buffer.buffer_ + size_, buffer_);
  }
  return *this;
}

AudioBuffer::AudioBuffer(AudioBuffer&& audio_buffer) noexcept
    : buffer_(nullptr), size_(0), pts_(0) {
  buffer_ = audio_buffer.buffer_;
  size_ = audio_buffer.size_;
  pts_ = audio_buffer.pts_;

  audio_buffer.buffer_ = nullptr;
  audio_buffer.size_ = 0;
  audio_buffer.pts_ = 0;
}


AudioBuffer& AudioBuffer::operator=(AudioBuffer&& audio_buffer) noexcept {
  if (this != &audio_buffer) {
    delete[] buffer_;
    buffer_ = nullptr;

    buffer_ = audio_buffer.buffer_;
    size_ = audio_buffer.size_;
    pts_ = audio_buffer.pts_;

    audio_buffer.buffer_ = nullptr;
    audio_buffer.size_ = 0;
    audio_buffer.pts_ = 0;
  }
  return *this;
}

uint8_t* AudioBuffer::buffer() const { return buffer_; }

uint64_t AudioBuffer::size() const { return size_; }

uint64_t AudioBuffer::pts() const { return pts_; }
