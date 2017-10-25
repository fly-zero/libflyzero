#pragma once

#include <arpa/inet.h>
#include <netinet/in.h>

#include <array>
#include <cstring>
#include <cassert>

namespace flyzero
{
    class IPAddressV4
    {
    public:
        typedef std::array<uint8_t, 4> ByteArray4;

        IPAddressV4(void)
        {
            memset(&addr_, 0, sizeof addr_);
        }

        explicit IPAddressV4(const char * ip)
        {
            assert(ip);
            if (1 != inet_pton(AF_INET, ip, &addr_.inAddr_))
                memset(&addr_, 0, sizeof addr_);
        }

        explicit IPAddressV4(struct in_addr addr)
        {
            addr_.inAddr_ = addr;
        }

        struct in_addr getInAddr(void) const
        {
            return addr_.inAddr_;
        }

        struct in_addr & inAddr(void)
        {
            return addr_.inAddr_;
        }

        const struct in_addr & inAddr(void) const
        {
            return addr_.inAddr_;
        }

        bool operator==(const IPAddressV4 & other) const
        {
            return addr_.inAddr_.s_addr == other.addr_.inAddr_.s_addr;
        }

        size_t hash(void) const
        {
            return addr_.inAddr_.s_addr;
        }

    private:
        union AddressStorage
        {
            static_assert(sizeof (struct in_addr) == sizeof (ByteArray4),
                "size of in_addr and ByteArray4 are different");
            struct in_addr inAddr_;
            ByteArray4 bytes_;
        } addr_;
    };
}
