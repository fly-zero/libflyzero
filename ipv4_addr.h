#pragma once

#include <arpa/inet.h>
#include <netinet/in.h>

#include <array>
#include <cstring>
#include <cassert>

namespace flyzero
{
    class ipv4_addr
    {
    public:
        typedef std::array<uint8_t, 4> byte_array;

        ipv4_addr()
        {
            memset(&addr_, 0, sizeof addr_);
        }

        explicit ipv4_addr(const char * ip)
        {
            assert(ip);
            if (1 != inet_pton(AF_INET, ip, &addr_.in_addr_))
                memset(&addr_, 0, sizeof addr_);
        }

        explicit ipv4_addr(struct in_addr const addr)
        {
            addr_.in_addr_ = addr;
        }

        struct in_addr & in_addr()
        {
            return addr_.in_addr_;
        }

        const struct in_addr & in_addr() const
        {
            return addr_.in_addr_;
        }

        bool operator==(const ipv4_addr & other) const
        {
            return addr_.in_addr_.s_addr == other.addr_.in_addr_.s_addr;
        }

        std::size_t hash() const
        {
            return addr_.in_addr_.s_addr;
        }

    private:
        union addr_storage
        {
            static_assert(sizeof (struct in_addr) == sizeof (byte_array),
                "size of in_addr and ByteArray4 are different");
            struct in_addr in_addr_;
            byte_array bytes_;
        } addr_;
    };
}
