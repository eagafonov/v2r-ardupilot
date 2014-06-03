#include "logger.h"

#include <iostream>

// extern const AP_HAL::HAL& hal;

logger::logger(LOG_LEVEL level)
:mLevel(level)
{

}

logger::~logger()
{
    std::cerr << mStream.str() << std::endl;
}

// logger& logger::operator<<(const char* s)
// {
//     bool first = mStream.str().empty();
//     mStream << s;
// 
//     if (first) {
//         mStream << ' ';
//     }
// 
//     return *this;
// }
// 
// 
logger& logger::operator<<(const std::string &s)
{
    bool first = mStream.str().empty();
    mStream << '"' << s << '"';

    if (first) {
        mStream << ' ';
    }

    return *this;
}
