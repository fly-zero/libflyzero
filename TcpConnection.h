#pragma once

#include "FileDescriptor.h"
#include "Epoll.h"

namespace flyzero
{

    class TcpConnection
        : public IEpoll
    {
    public:
        constexpr TcpConnection(void) = default;

        explicit TcpConnection(const FileDescriptor & sock) noexcept
            : sock_(sock)
        {
            sock_.setNonblocking();
        }

        explicit TcpConnection(FileDescriptor && sock) noexcept
            : sock_(std::move(sock))
        {
            sock_.setNonblocking();
        }

        TcpConnection(const TcpConnection &) = default;

        TcpConnection(TcpConnection &&) = default;

        virtual ~TcpConnection(void) = default;

        int getFileDescriptor(void) const noexcept override
        {
            return sock_.get();
        }

        bool operator<(const TcpConnection & other) const noexcept
        {
            return sock_ < other.sock_;
        }

    private:
        FileDescriptor sock_;
    };

}