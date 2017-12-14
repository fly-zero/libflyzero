#pragma once

#include <array>
#include <cstring>
#include <cstdint>
#include <netinet/in.h>

namespace flyzero
{
    class ipv6_addr
    {
    public:
        typedef std::array<uint8_t, 16> byte_array;
        typedef std::array<uint64_t, 2> qword_array;

        const struct in6_addr & get_in_addr() const
        {
            return addr_.in_addr;
        }

        bool operator==(const ipv6_addr & other) const
        {
            return (addr_.qwords[0] == other.addr_.qwords[0] && addr_.qwords[1] == other.addr_.qwords[1]);
        }

        std::size_t hash() const
        {
#if (defined _WIN64) || (defined __x86_64__)
            static_assert(sizeof (std::size_t) == sizeof (uint64_t),
                "sizeof 'size_t' and 'uint64_t' are different");
            return addr_.qwords[0] ^ addr_.qwords[1];
#else
            static_assert(sizeof (size_t) == sizeof (uint32_t),
                "sizeof 'size_t' and 'uint32_t' are different");
            auto & words = addr.inAddr.__in6_u.__u6_addr32;
            return words[0] ^ words[1] ^ words[2] ^ words[3];
#endif
        }

    private:
        union AddressStorage
        {
            static_assert(sizeof (struct in6_addr) == sizeof (byte_array),
                "size of 'in6_addr' and 'ByteArray16' are different");
            struct in6_addr in_addr;
            byte_array bytes;
            qword_array qwords;
            AddressStorage(void)
            {
                memset(this, 0, sizeof (AddressStorage));
            }
        } addr_;
    };
}
