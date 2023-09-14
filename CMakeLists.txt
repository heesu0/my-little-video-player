# Minimum CMake version and project information
cmake_minimum_required(VERSION 3.14)
project(my-little-video-player LANGUAGES C CXX)

# Set C++ standard and build type
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_BUILD_TYPE Debug)

# Compiler flags
add_compile_options(-Wall -Wextra -Wpedantic)

# Pthread preference
set(THREADS_PREFER_PTHREAD_FLAG ON)

# Add external libraries
add_subdirectory(lib/ffmpeg)
add_subdirectory(lib/sdl2)
find_package(Threads REQUIRED)

# Source files
set(SOURCES
  src/log.cpp
  src/demuxer.cpp
  src/decoder.cpp
  src/video_converter.cpp
  src/video_renderer.cpp
  src/video_player.cpp
  src/audio_resampler.cpp
  src/audio_renderer.cpp
  src/audio_buffer.cpp
)

# List of examples and their source files
set(EXAMPLES
  log_example
  demuxer_example
  decoder_example
  video_converter_example
  video_renderer_example
  audio_resampler_example
  audio_renderer_example
)

# Generate executables for each example
foreach(EXAMPLE ${EXAMPLES})
  add_executable(${EXAMPLE} example/${EXAMPLE}.cpp ${SOURCES})
  target_link_libraries(${EXAMPLE} ffmpeg sdl2 Threads::Threads)
endforeach()

# Main video player executable
add_executable(video_player src/main.cpp ${SOURCES})
target_link_libraries(video_player ffmpeg sdl2 Threads::Threads)
