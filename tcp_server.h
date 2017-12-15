#pragma once

#include <netinet/in.h>
#include <arpa/inet.h>

#include <memory>
#include <cassert>
#include <functional>

#include "file_descriptor.h"
#include "epoll.h"
#include "TcpConnection.h"

namespace flyzero
{

    class tcp_server
        : public epoll_listener
    {
    public:
        using alloc_type = std::function<void*(std::size_t)>;
        using dealloc_type = std::function<void(void *)>;

        tcp_server() = default;

        tcp_server(const alloc_type & alloc, const dealloc_type & dealloc)
            : epoll_(alloc, dealloc)
        {
            assert(alloc);
            assert(dealloc);
        }

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

        file_descriptor accept() const
        {
            sockaddr addr;
            auto len = socklen_t(sizeof addr);
            return file_descriptor(::accept(sock_.get(), &addr, &len));
        }

        file_descriptor accept(sockaddr_in & addr) const
        {
            auto len = socklen_t(sizeof addr);
            return file_descriptor(::accept(sock_.get(), reinterpret_cast<sockaddr *>(&addr), &len));
        }

        void close()
        {
            sock_.close();
        }

        int get_fd() const override
        {
            return sock_.get();
        }

        void run(const std::size_t size, const int timeout)
        {
            epoll_.run(size, timeout, on_timeout, this);
        }

        static void on_timeout(void * server)
        {
            static_cast<tcp_server *>(server)->on_timeout();
        }

        virtual void on_timeout() = 0;

        void add_connection(TcpConnection & connection, const uint32_t events)
        {
            epoll_.add(connection, events);
        }

        void remove_connection(const TcpConnection & connection)
        {
            epoll_.remove(connection);
        }

        void on_write() override { }

        void on_close() override { }

    private:
        file_descriptor sock_;
        epoll epoll_;
    };

}