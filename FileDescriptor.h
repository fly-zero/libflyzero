#pragma once

#include <unistd.h>
#include <fcntl.h>

namespace flyzero
{

    class FileDescriptor
    {
    public:
        FileDescriptor(void) = default;

        explicit FileDescriptor(int fd)
            : fd_(fd)
        {
        }

        FileDescriptor(const FileDescriptor & other)
            : fd_(other.fd_ == -1 ? -1 : ::dup(other.fd_))
        {
        }

        FileDescriptor(FileDescriptor && other)
            : fd_(other.fd_)
        {
            other.fd_ = -1;
        }

        ~FileDescriptor(void)
        {
            if (fd_ >= 0) ::close(fd_);
        }

        int get(void) const
        {
            return fd_;
        }

        void close(void)
        {
            ::close(fd_);
            fd_ = -1;
        }

        FileDescriptor Dup(void)
        {
            return FileDescriptor(::dup(fd_));
        }

        bool SetUnblock(void) const
        {
            auto ret = fcntl(fd_, F_GETFL);
            if (ret == -1)
                return false;
            return fcntl(fd_, F_SETFL, ret | O_NONBLOCK) != -1;
        }

        bool operator!(void) const
        {
            return fd_ == -1;
        }

        operator bool(void) const
        {
            return fd_ != -1;
        }

        FileDescriptor & operator=(const FileDescriptor & other)
        {
            if (this != &other)
                fd_ = other.fd_ == -1 ? -1 : ::dup(other.fd_);
            return *this;
        }

        FileDescriptor & operator=(FileDescriptor && other)
        {
            if (this != &other)
            {
                fd_ = other.fd_;
                other.fd_ = -1;
            }
            return *this;
        }

    private:
        int fd_{ -1 };
    };

}