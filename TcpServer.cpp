#include <sys/socket.h>
#include <netinet/in.h>

#include "TcpServer.h"

namespace flyzero
{

    bool TcpServer::listen(unsigned short port)
    {
        FileDescriptor sock(::socket(AF_INET, SOCK_STREAM, 0));

        if (!sock)
            return false;

        if (!sock.setNonblocking())
            return false;

        sockaddr_in addr{ };
        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = INADDR_ANY;
        addr.sin_port = htons(port);

        if (::bind(sock.get(), reinterpret_cast<sockaddr *>(&addr), sizeof addr) == -1)
            return false;

        if (::listen(sock.get(), 1024) == -1)
            return false;

        sock_ = std::move(sock);
        epoll_.add(*this, flyzero::epoll::epoll_read | flyzero::epoll::epoll_edge);
        return true;
    }
}
