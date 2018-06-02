#pragma once

#include <arpa/inet.h>

#include <memory>
#include <cassert>
#include <functional>

#include <FileDescriptor.h>
#include <EventBase.h>

namespace flyzero
{

    class tcp_server
        : public EventListener
    {
    public:
        tcp_server(void) = default;

        tcp_server(const tcp_server &) = default;

        tcp_server(tcp_server &&) = default;

        virtual ~tcp_server(void) = default;

        tcp_server & operator=(const tcp_server &) = default;

        tcp_server & operator=(tcp_server &&) = default;

        // listen on 0.0.0.0:port
        bool listen(const unsigned short port);

        FileDescriptor accept(sockaddr_storage & addr, socklen_t & addrlen) const
        {
            assert(sizeof addr == addrlen);

            return FileDescriptor(::accept4(sock_.Get(), reinterpret_cast<sockaddr *>(&addr), &addrlen, SOCK_NONBLOCK));
        }

        void close(void) { sock_.Close(); }

        virtual void on_accept(FileDescriptor && sock, const sockaddr_storage & addr, socklen_t addrlen) = 0;

        bool OnRead(void) override final;

        bool OnWrite(void) override final { return true; }

        void OnClose(void) override final { }

        int GetFd(void) const override final { return sock_.Get(); }

    private:
        FileDescriptor sock_;
    };

}
