#pragma once

#include <netinet/in.h>

#include <memory>
#include <cstdlib>
#include <cassert>
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
        typedef void * (*AllocFunc)(size_t);
        typedef void (*DeallocFunc)(void *);

        TcpServer(void) = default;

        TcpServer(AllocFunc alloc, DeallocFunc dealloc)
            : alloc_(alloc)
            , dealloc_(dealloc)
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
        AllocFunc alloc_{ ::malloc };
        DeallocFunc dealloc_{ ::free };
    };


    template<class _Type, class _Alloc, class _Dealloc>
    class TcpServerAllocator
        : public Allocator<_Type, _Alloc, _Dealloc>
    {
    public:
        TcpServerAllocator(TcpServer & tcpServer)
            : tcpServer_(tcpServer)
        {
        }

    private:
        TcpServer & tcpServer_;
    };

}