#include "../src/demuxer.h"

#include <iostream>
#include <memory>

int main(int argc, char** argv)
{
    try
    {
        if (argc != 2)
        {
            throw std::logic_error("Invalid arguments");
        }

        auto demuxer = std::make_unique<Demuxer>(argv[1]);
        demuxer->Init();
        demuxer->PrintFileInfo();

        std::shared_ptr<AVPacket> packet;
        demuxer->GetPacket(packet);
    }
    catch (const std::exception& e)
    {
        std::cerr << e.what() << std::endl;
        return -1;
    }

    return 0;
}
