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

protected:
    virtual void on_accept(file_descriptor &&sock, const sockaddr_storage &addr,
                           socklen_t addrlen) = 0;

protected:
    void on_read(void) override final;

    void on_write(void) override final {}

    static int listen(unsigned short port);

    static int listen(const char *unix_path);
};

inline TcpServer::TcpServer(int sock) : IoListener(sock) {}

}  // namespace flyzero
