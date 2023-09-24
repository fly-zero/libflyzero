#include "tcp_server.h"

#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/un.h>

#include <cerrno>

#include "file_descriptor.h"
#include "utility.h"

namespace flyzero {

void TcpServer::on_read(void) {
    while (true) {
        sockaddr_storage addr{};
        socklen_t addrlen = sizeof addr;
        file_descriptor sock{::accept4(fd(),
                                       reinterpret_cast<sockaddr *>(&addr),
                                       &addrlen, SOCK_NONBLOCK)};

        if (sock)
            on_accept(std::move(sock), addr, addrlen);
        else if (errno == EAGAIN || errno == EWOULDBLOCK) {
            break;
        } else {
            throw utility::system_error(
                errno, "accept4(%d, %p, %p, SOCK_NONBLOCK) failed: %s", fd(),
                &addr, &addrlen, std::strerror(errno));
        }
    }
}

int TcpServer::listen(const unsigned short port) {
    // 创建套接字
    file_descriptor sock(::socket(AF_INET, SOCK_STREAM, 0));
    if (!sock) return -1;

    // 设置非阻塞
    if (!sock.set_nonblocking()) return -1;

    // 绑定地址
    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);

    auto err =
        ::bind(sock.get(), reinterpret_cast<sockaddr *>(&addr), sizeof addr);
    if (err != 0) return -1;

    // 监听
    err = ::listen(sock.get(), 1024);
    if (err != 0) return -1;

    return sock.release();
}

int TcpServer::listen(const char *const unix_path) {
    // 创建套接字
    file_descriptor sock(::socket(AF_UNIX, SOCK_STREAM, 0));
    if (!sock) return -1;

    // 设置非阻塞
    if (!sock.set_nonblocking()) return -1;

    // 删除已存在的文件
    auto err = ::unlink(unix_path);
    if (err != 0 && errno != ENOENT) return -1;

    // 绑定地址
    sockaddr_un addr{};
    addr.sun_family = AF_UNIX;
    std::strcpy(addr.sun_path, unix_path);

    err = ::bind(sock.get(), reinterpret_cast<sockaddr *>(&addr), sizeof addr);
    if (err != 0) return -1;

    // 监听
    err = ::listen(sock.get(), 1024);
    if (err != 0) return -1;

    return sock.release();
}

}  // namespace flyzero
