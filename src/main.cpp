#include "video_player.h"

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

        auto player = std::make_unique<VideoPlayer>(argv[1]);
        player->Init();
        player->Run();
    }
    catch (const std::exception& e)
    {
        std::cerr << e.what() << std::endl;
        return -1;
    }

    return 0;
}
