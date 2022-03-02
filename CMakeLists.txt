CMAKE_MINIMUM_REQUIRED(VERSION 3.14)
PROJECT(my-little-video-player C CXX)

SET(CMAKE_CXX_STANDARD 14)
SET(CMAKE_BUILD_TYPE Debug)
set(CMAKE_CXX_FLAGS "-Wall")

ADD_SUBDIRECTORY(lib/ffmpeg)
ADD_SUBDIRECTORY(lib/sdl2)

LIST(APPEND sources
  example/log_example.cpp
  src/log.cpp
  )

ADD_EXECUTABLE(video-player ${sources})
TARGET_LINK_LIBRARIES(video-player ffmpeg sdl2)