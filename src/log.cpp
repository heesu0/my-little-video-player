#include "log.h"

extern "C" {
#include <libavutil/avutil.h>
}

#include <SDL.h>
#include <array>
#include <stdexcept>

namespace {

constexpr size_t ERROR_SIZE = 128;

std::string getMessage(const int error_code)
{
    std::array<char, ERROR_SIZE> error_buffer{};
    av_make_error_string(error_buffer.data(), ERROR_SIZE, error_code);
    return {error_buffer.data()};
}

} // namespace

void LOG::ERROR(const std::string& message)
{
    throw std::runtime_error("ERROR LOG : " + message);
}

void LOG::ERROR_FROM_FFMPEG(int error_code)
{
    std::string ffmpeg_error = getMessage(error_code);
    throw std::runtime_error("ERROR LOG : " + ffmpeg_error);
}

void LOG::ERROR_FROM_SDL()
{
    std::string sdl_error = SDL_GetError();
    throw std::runtime_error("ERROR LOG : " + sdl_error);
}
