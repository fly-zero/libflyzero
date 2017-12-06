#include "Memory.h"

namespace flyzero
{

    unsigned int Memory::SearchCharacters(const void * src, unsigned int srclen, const char * str, unsigned int strlen)
    {
        for (unsigned int i = 0; i < srclen; ++i)
            for (unsigned int j = 0; j < strlen; ++j)
                if (((char *)src)[i] == str[j])
                    return i;
        return static_cast<unsigned int>(-1);
    }

    uint64_t Memory::ReverseByteOrder(uint64_t val)
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

    uint32_t Memory::ReverseByteOrder(uint32_t val)
    {
        return (((val & 0xFF000000) >> 24)
            | ((val & 0x00FF0000) >> 8)
            | ((val & 0x0000FF00) << 8)
            | ((val & 0x000000FF) << 24));
    }

}
