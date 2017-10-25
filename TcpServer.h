#pragma once

#include <netinet/in.h>

#include <memory>
#include <cstdlib>
#include <cassert>
#include <functional>
#include <set>

#include "FileDescriptor.h"
#include "Epoll.h"
#include "Allocator.h"

namespace flyzero
{

    class TcpServer
        : public IEpoll
    {
    public:
        using alloc_type = std::function<void*(size_t)>;
        using dealloc_type = std::function<void(void *)>;
        using pointer = std::unique_ptr<void *, dealloc_type>;
        using allocator = Allocator<pointer, alloc_type, dealloc_type>;
        using set_type = std::set<pointer, std::less<pointer>, allocator>;

        TcpServer(void) = default;

        TcpServer(alloc_type alloc, dealloc_type dealloc)
            : alloc_(alloc)
            , dealloc_(dealloc)
            , set_(set_type::key_compare(), allocator(alloc_, dealloc_))
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

        TcpServer & operator=(TcpServer && other)
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
            socklen_t len = sizeof addr;
            return FileDescriptor(::accept(sock_.get(), &addr, &len));
        }

        FileDescriptor accept(sockaddr_in & addr) const
        {
            socklen_t len = sizeof addr;
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

    private:
        FileDescriptor sock_;
        alloc_type alloc_{ ::malloc };
        dealloc_type dealloc_{ ::free };
        set_type set_;
    };

}