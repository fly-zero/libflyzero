#include "tcp_server.h"

#include <netinet/in.h>
#include <sys/socket.h>

namespace flyzero {

int TcpServer::listen(const unsigned short port) {
    file_descriptor sock(::socket(AF_INET, SOCK_STREAM, 0));

    if (!sock) return false;

    if (!sock.set_nonblocking()) return false;

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);

    auto err =
        ::bind(sock.get(), reinterpret_cast<sockaddr *>(&addr), sizeof addr);
    if (err != 0) return -1;

    err = ::listen(sock.get(), 1024);
    if (err != 0) return -1;

    return sock.release();
}

void TcpServer::on_read(void) {
    sockaddr_storage addr;  // NOLINT
    socklen_t addrlen = sizeof addr;

    auto sock = accept(addr, addrlen);

    if (sock) {
        on_accept(std::move(sock), addr, addrlen);
    }
}
}  // namespace flyzero
