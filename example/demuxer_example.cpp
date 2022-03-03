#include "../src/demuxer.h"
#include <iostream>
#include <memory>

int main(int argc, char** argv) {
  try {
    if (argc != 2) {
      throw std::logic_error("Invalid arguments");
    }

    std::unique_ptr<Demuxer> demuxer = std::make_unique<Demuxer>(argv[1]);
    demuxer->init();
    demuxer->printFileInfo();

    std::shared_ptr<AVPacket> packet;
    demuxer->getPacket(packet);
  } catch (const std::exception& e) {
    std::cerr << e.what() << std::endl;
    return -1;
  }

  return 0;
}