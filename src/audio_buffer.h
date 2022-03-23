#pragma once
#include <cstdint>

typedef struct AudioBuffer {
  uint8_t* buffer;
  unsigned int size;
} AudioBuffer;
