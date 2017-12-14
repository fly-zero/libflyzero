#pragma once

#include <cstdint>	// c++11 is needed
#include "Error.h"

namespace flyzero
{
    class hex
    {
    public:
        static ErrorCode hex_str(const char *str, unsigned int length, uint64_t &val);
        static ErrorCode hex_str(const char *str, unsigned int length, uint32_t &val);
        static ErrorCode hex_str(const char *str, unsigned int length, uint8_t &val);
        static ErrorCode hex_str(const char *str, unsigned int length, void * buffer, unsigned int & size);
    };
}
