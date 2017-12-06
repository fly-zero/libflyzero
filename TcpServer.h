#pragma once

#include <netinet/in.h>

#include <memory>
#include <cassert>
#include <functional>

#include "FileDescriptor.h"
#include "Epoll.h"
#include "TcpConnection.h"

namespace flyzero
{

    class TcpServer
        : public IEpoll
    {
    public:
        using alloc_type = std::function<void*(size_t)>;
        using dealloc_type = std::function<void(void *)>;

        TcpServer(void) = default;

        TcpServer(const alloc_type & alloc, const dealloc_type & dealloc)
            : epoll_(alloc, dealloc)
        {
            assert(alloc);
            assert(dealloc);
        }

        TcpServer(const TcpServer &) = default;

        TcpServer(TcpServer &&) = default;

        virtual ~TcpServer(void) = default;

        TcpServer & operator=(const TcpServer & other)
        {
            if (this != &other)
                sock_ = other.sock_;
            return *this;
        }

        TcpServer & operator=(TcpServer && other) noexcept
        {
            if (this != &other)
                sock_ = std::move(other.sock_);
            return *this;
        }

        // listen on 0.0.0.0:port
        bool listen(unsigned short port);

        FileDescriptor accept(void) const
        {
            sockaddr addr;
            auto len = socklen_t(sizeof addr);
            return FileDescriptor(::accept(sock_.get(), &addr, &len));
        }

        FileDescriptor accept(sockaddr_in & addr) const
        {
            auto len = socklen_t(sizeof addr);
            return FileDescriptor(::accept(sock_.get(), reinterpret_cast<sockaddr *>(&addr), &len));
        }

        void close(void)
        {
            sock_.close();
        }

        int getFileDescriptor(void) const override
        {
            return sock_.get();
        }

        void run(size_t size, int timeout)
        {
            epoll_.run(size, timeout, onTimeout, this);
        }

        static void onTimeout(void * tcpServer)
        {
            static_cast<TcpServer *>(tcpServer)->onTimeout();
        }

        virtual void onTimeout(void) = 0;

        void addConnection(TcpConnection & connection, uint32_t events)
        {
            epoll_.add(connection, events);
        }

        void removeConnection(const TcpConnection & connection)
        {
            epoll_.remove(connection);
        }

        void onWrite(void) override { }

        void onClose(void) override { }

    private:
        FileDescriptor sock_;
        Epoll epoll_;
    };

}