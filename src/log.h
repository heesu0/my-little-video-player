#ifndef __LOG_H__
#define __LOG_H__

#include <string>

class LOG
{
public:
    static void ERROR(const std::string& message);
    static void ERROR_FROM_FFMPEG(int error_code);
    static void ERROR_FROM_SDL();
};

#endif
