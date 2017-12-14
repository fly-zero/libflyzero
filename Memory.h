#pragma once

#include <cstdint>		// c++11 is needed

namespace flyzero
{
    class memory
    {
    public:
        static uint64_t reverse_byte_order(const uint64_t val);
        static uint32_t reverse_byte_order(const uint32_t val);
    };
}
