# Minimum required version and project information
cmake_minimum_required(VERSION 3.14)
project(ffmpeg)

# Ensure the availability of the PkgConfig module
find_package(PkgConfig REQUIRED)

# Search for ffmpeg libraries using PkgConfig
pkg_check_modules(AVCODEC REQUIRED IMPORTED_TARGET libavcodec)
pkg_check_modules(AVFORMAT REQUIRED IMPORTED_TARGET libavformat)
pkg_check_modules(AVFILTER REQUIRED IMPORTED_TARGET libavfilter)
pkg_check_modules(AVDEVICE REQUIRED IMPORTED_TARGET libavdevice)
pkg_check_modules(AVUTIL REQUIRED IMPORTED_TARGET libavutil)
pkg_check_modules(SWRESAMPLE REQUIRED IMPORTED_TARGET libswresample)
pkg_check_modules(SWSCALE REQUIRED IMPORTED_TARGET libswscale)

# Define an interface library to group the ffmpeg libraries
add_library(ffmpeg INTERFACE IMPORTED GLOBAL)

# Link the discovered ffmpeg libraries to the interface library
target_link_libraries(ffmpeg INTERFACE
  PkgConfig::AVCODEC
  PkgConfig::AVFORMAT
  PkgConfig::AVFILTER
  PkgConfig::AVDEVICE
  PkgConfig::AVUTIL
  PkgConfig::SWRESAMPLE
  PkgConfig::SWSCALE
)
