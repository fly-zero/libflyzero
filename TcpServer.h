#pragma once

#include <memory>

#include "FileDescriptor.h"

namespace flyzero
{

    class TcpServer
    {
    public:
        TcpServer(void) = default;
        
        TcpServer(const TcpServer & other) = default;

        TcpServer(TcpServer && other)
            : sock_(std::move(other.sock_))
        {
        }

        ~TcpServer(void) = default;

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

        void close(void)
        {
            sock_.close();
        }

    private:
        FileDescriptor sock_;
    };

}