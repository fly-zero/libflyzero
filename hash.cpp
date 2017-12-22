#include "hash.h"

namespace flyzero
{

    std::size_t hash_bytes(const void * bytes, const std::size_t size)
    {
        const auto n = size / sizeof (std::size_t);
        const auto r = size % sizeof (std::size_t);
        const auto p = reinterpret_cast<const std::size_t *>(bytes);

        std::size_t res = 0;

        for (std::size_t i = 0; i < n; ++i)
            res ^= p[i];

        for (std::size_t i = 0; i < r; ++i)
            res ^= reinterpret_cast<const unsigned char *>(p + n)[i] << (i << 3);

        return res;
    }

}
