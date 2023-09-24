#pragma once

#include <arpa/inet.h>

#include <cassert>
#include <functional>
#include <memory>

#include "event_dispatch.h"
#include "file_descriptor.h"

namespace flyzero {

class TcpServer : public EventDispatch::IoListener {
public:
    TcpServer(int sock);

    TcpServer(const TcpServer &) = default;

    TcpServer(TcpServer &&) = default;

    virtual ~TcpServer(void) = default;

    TcpServer &operator=(const TcpServer &) = default;

    TcpServer &operator=(TcpServer &&) = default;

    file_descriptor accept(sockaddr_storage &addr, socklen_t &addrlen) const {
        assert(sizeof addr == addrlen);

        return file_descriptor(::accept4(fd(),
                                         reinterpret_cast<sockaddr *>(&addr),
                                         &addrlen, SOCK_NONBLOCK));
    }

    virtual void on_accept(file_descriptor &&sock, const sockaddr_storage &addr,
                           socklen_t addrlen) = 0;

protected:
    void on_read(void) override final;

    void on_write(void) override final {}

    static int listen(unsigned short port);
};

inline TcpServer::TcpServer(int sock) : IoListener(sock) {}

}  // namespace flyzero
