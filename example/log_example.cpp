#include "../src/log.h"
#include <iostream>
#include <string>

int main(int argc, char** argv) {
  try {
    LOG::Error("error message..");
    // LOG::Error(-1);
  } catch (const std::exception& e) {
    std::cerr << e.what() << std::endl;
    return -1;
  }

  return 0;
}
