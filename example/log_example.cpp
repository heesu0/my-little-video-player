#include "../src/log.h"
#include <iostream>

int main(int argc, char** argv) {
  try {
    std::cout << "Hello World\n";
    LOG::Error("error message..");
    // LOG::Error(-1);
  } catch (const std::exception& e) {
    std::cerr << e.what() << std::endl;
    return -1;
  }

  return 0;
}