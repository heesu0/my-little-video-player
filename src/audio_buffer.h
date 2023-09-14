#ifndef __AUDIO_BUFFER_H__
#define __AUDIO_BUFFER_H__

#include <cstdint>

class AudioBuffer
{
public:
    explicit AudioBuffer(uint64_t size, uint64_t pts);
    ~AudioBuffer();
    AudioBuffer(const AudioBuffer& audio_buffer);
    AudioBuffer& operator=(const AudioBuffer& audio_buffer);
    AudioBuffer(AudioBuffer&& audio_buffer) noexcept;
    AudioBuffer& operator=(AudioBuffer&& audio_buffer) noexcept;

    uint8_t* buffer() const;
    uint64_t size() const;
    uint64_t pts() const;

private:
    uint8_t* buffer_;
    uint64_t size_;
    uint64_t pts_;
};

#endif
