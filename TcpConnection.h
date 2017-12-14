#pragma once

#include "file_descriptor.h"
#include "epoll.h"

namespace flyzero
{

    class TcpConnection
        : public epoll_listener
    {
    public:
        constexpr TcpConnection(void) = default;

        explicit TcpConnection(const file_descriptor & sock) noexcept
            : sock_(sock)
        {
            sock_.set_nonblocking();
        }

        explicit TcpConnection(file_descriptor && sock) noexcept
            : sock_(std::move(sock))
        {
            sock_.set_nonblocking();
        }

        TcpConnection(const TcpConnection &) = default;

        TcpConnection(TcpConnection &&) = default;

        virtual ~TcpConnection(void) = default;

        int get_fd(void) const noexcept override
        {
            return sock_.get();
        }

        bool operator<(const TcpConnection & other) const noexcept
        {
            return sock_ < other.sock_;
        }

    private:
        file_descriptor sock_;
    };

}
