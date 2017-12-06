#pragma once

#include <array>
#include <cstring>
#include <netinet/in.h>

namespace flyzero
{
    class IPAddressV6
    {
    public:
        typedef std::array<uint8_t, 16> ByteArray16;
        typedef std::array<uint64_t, 2> QwordArray2;

        const struct in6_addr getInAddr(void) const
        {
            return addr.inAddr;
        }

        bool operator==(const IPAddressV6 & other) const
        {
            return (addr.qwords[0] == other.addr.qwords[0] && addr.qwords[1] == other.addr.qwords[1]);
        }

        size_t hash(void) const
        {
#if (defined _WIN64) || (defined __x86_64__)
            static_assert(sizeof (size_t) == sizeof (uint64_t),
                "sizeof 'size_t' and 'uint64_t' are different");
            return addr.qwords[0] ^ addr.qwords[1];
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
            static_assert(sizeof (struct in6_addr) == sizeof (ByteArray16),
                "size of 'in6_addr' and 'ByteArray16' are different");
            struct in6_addr inAddr;
            ByteArray16 bytes;
            QwordArray2 qwords;
            AddressStorage(void)
            {
                memset(this, 0, sizeof (AddressStorage));
            }
        } addr;
    };
}
