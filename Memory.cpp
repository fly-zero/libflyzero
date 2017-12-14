#include "memory.h"

namespace flyzero
{

    uint64_t memory::reverse_byte_order(const uint64_t val)
    {
        return (((val & 0xFF00000000000000) >> 56)
            | ((val & 0x00FF000000000000) >> 40)
            | ((val & 0x0000FF0000000000) >> 24)
            | ((val & 0x000000FF00000000) >> 8)
            | ((val & 0x00000000FF000000) << 8)
            | ((val & 0x0000000000FF0000) << 24)
            | ((val & 0x000000000000FF00) << 40)
            | ((val & 0x00000000000000FF) << 56));
    }

    uint32_t memory::reverse_byte_order(const uint32_t val)
    {
        return (((val & 0xFF000000) >> 24)
            | ((val & 0x00FF0000) >> 8)
            | ((val & 0x0000FF00) << 8)
            | ((val & 0x000000FF) << 24));
    }

}
