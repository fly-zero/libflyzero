#pragma once

#include <arpa/inet.h>

#include <memory>
#include <cassert>
#include <functional>

#include "file_descriptor.h"
#include "epoll.h"

namespace flyzero
{

    class tcp_server
        : public epoll_listener
    {
    public:
        tcp_server() = default;

        tcp_server(const tcp_server &) = default;

        tcp_server(tcp_server &&) = default;

        virtual ~tcp_server() = default;

        tcp_server & operator=(const tcp_server & other)
        {
            if (this != &other)
                sock_ = other.sock_;
            return *this;
        }

        tcp_server & operator=(tcp_server && other) noexcept
        {
            if (this != &other)
                sock_ = std::move(other.sock_);
            return *this;
        }

        // listen on 0.0.0.0:port
        bool listen(const unsigned short port);

        file_descriptor accept(sockaddr_storage & addr, socklen_t & addrlen) const
        {
            assert(sizeof addr == addrlen);

            return file_descriptor(::accept4(sock_.get(), reinterpret_cast<sockaddr *>(&addr), &addrlen, SOCK_NONBLOCK));
        }

        void close() { sock_.close(); }

        virtual void on_accept(file_descriptor && sock, const sockaddr_storage & addr, socklen_t addrlen) = 0;

        void on_read() override final
        {
            sockaddr_storage addr;  // NOLINT
            socklen_t addrlen = sizeof addr;
            auto sock = accept(addr, addrlen);
            if (sock)
                on_accept(std::move(sock), addr, addrlen);
        }

        void on_write() override final { }

        void on_close() override final { }

        int get_fd() const override final { return sock_.get(); }

    private:
        file_descriptor sock_;
    };

}
