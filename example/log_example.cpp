#include "../src/log.h"
#include <iostream>
#include <string>

int main(int argc, char** argv) {
  std::cout << argc << argv << std::endl;
  try {
    LOG::ERROR("error message..");
    // LOG::ERROR_FROM_FFMPEG(-1);
  } catch (const std::exception& e) {
    std::cerr << e.what() << std::endl;
    return -1;
  }

  return 0;
}
