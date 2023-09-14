# Minimum required version and project information
cmake_minimum_required(VERSION 3.14)
project(sdl2)

# Ensure the availability of the PkgConfig module
find_package(PkgConfig REQUIRED)

# Search for SDL2 using PkgConfig
pkg_check_modules(SDL2 REQUIRED IMPORTED_TARGET sdl2)

# Define an interface library to group the SDL2 libraries
add_library(sdl2 INTERFACE IMPORTED GLOBAL)

# Link the discovered SDL2 libraries to the interface library
target_link_libraries(sdl2 INTERFACE PkgConfig::SDL2)
