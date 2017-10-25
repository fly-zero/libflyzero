#pragma once

#include <netinet/in.h>

#include <memory>
#include <cstdlib>
#include <cassert>
#include <functional>
#include <set>

#include "Allocator.h"
#include "FileDescriptor.h"
#include "Epoll.h"
#include "TcpConnection.h"

namespace flyzero
{

    class TcpServer
        : public IEpoll
    {
    protected:
        class connection_deleter
        {
        public:
            using dealloc_type = std::function<void(void *)>;
            
            constexpr connection_deleter(void) = default;

            connection_deleter(const dealloc_type & dealloc)
                : dealloc_(dealloc)
            {
            }

            void operator()(TcpConnection * connection)
            {
                assert(connection);
                connection->~TcpConnection();
                dealloc_(connection);
            }

        private:
            dealloc_type dealloc_{ ::free };
        };

    public:
        using alloc_type = std::function<void*(size_t)>;
        using dealloc_type = std::function<void(void *)>;
        using conneciton_pointer = std::unique_ptr<TcpConnection, connection_deleter>;
        using connection_allocator = Allocator<conneciton_pointer, alloc_type, dealloc_type>;
        using connection_set = std::set<conneciton_pointer, std::less<conneciton_pointer>, connection_allocator>;

        TcpServer(void) = default;

        TcpServer(const alloc_type & alloc, const dealloc_type & dealloc)
            : set_(connection_set::key_compare(), connection_allocator(alloc, dealloc))
            , epoll_(alloc, dealloc)
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

        void run(size_t size, int timeout)
        {
            epoll_.run(size, timeout, onTimeout, this);
        }

        static void onTimeout(void * tcpServer)
        {
            static_cast<TcpServer *>(tcpServer)->onTimeout();
        }

        virtual void onTimeout(void) = 0;

        void onRead(void) override;

        void onWrite(void) override { }

        void onClose(void) override { }

        void add(TcpConnection * connection, const dealloc_type & dealloc)
        {
            epoll_.add(connection, Epoll::Event::READ | Epoll::EDGE);
            //set_.emplace(connection, dealloc);
            conneciton_pointer p(connection, connection_deleter(dealloc));
        }

    private:
        FileDescriptor sock_;
        connection_set set_;
        Epoll epoll_;
    };

}