#include "audio_buffer.h"

AudioBuffer::AudioBuffer(int size) : size_(size) {
  buffer_ = new uint8_t[size_];
}

AudioBuffer::~AudioBuffer() { delete[] buffer_; }

uint8_t* AudioBuffer::buffer() const { return buffer_; }

int AudioBuffer::size() const { return size_; }
