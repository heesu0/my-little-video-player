#include "log.h"
#include <array>
#include <stdexcept>
extern "C" {
#include "libavutil/avutil.h"
}

namespace {

constexpr size_t ERROR_SIZE = 128;

std::string getMessage(const int error_code) {
  std::array<char, ERROR_SIZE> error_buffer;
  av_make_error_string(error_buffer.data(), ERROR_SIZE, error_code);
  return std::string(error_buffer.data());
}

}// namespace

void LOG::Error(const std::string& message) {
  throw std::runtime_error("LOG : " + message);
}

void LOG::Error(int error_code) {
  throw std::runtime_error("LOG : " + getMessage(error_code));
}