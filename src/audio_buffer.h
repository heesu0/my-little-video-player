#pragma once
#include <cstdint>

class AudioBuffer {
 public:
  explicit AudioBuffer(int size);
  ~AudioBuffer();

  AudioBuffer(const AudioBuffer& audio_buffer);
  AudioBuffer& operator=(const AudioBuffer& audio_buffer);

  AudioBuffer(AudioBuffer&& audio_buffer) noexcept;
  AudioBuffer& operator=(AudioBuffer&& audio_buffer) noexcept;

  uint8_t* buffer() const;
  int size() const;

 private:
  uint8_t* buffer_;
  int size_;
};
