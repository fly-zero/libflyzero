#pragma once

#include <unistd.h>
#include <fcntl.h>

namespace flyzero
{

    class FileDescriptor
    {
    public:
        constexpr FileDescriptor(void) = default;

        explicit FileDescriptor(int fd) noexcept
            : fd_(fd)
        {
        }

        FileDescriptor(const FileDescriptor & other) noexcept
            : fd_(other.fd_ == -1 ? -1 : ::dup(other.fd_))
        {
        }

        FileDescriptor(FileDescriptor && other) noexcept
            : fd_(other.fd_)
        {
            other.fd_ = -1;
        }

        ~FileDescriptor(void) noexcept
        {
            if (fd_ >= 0) ::close(fd_);
        }

        int get(void) const noexcept
        {
            return fd_;
        }

        void close(void) noexcept
        {
            ::close(fd_);
            fd_ = -1;
        }

        FileDescriptor clone(void) const noexcept
        {
            return FileDescriptor(::dup(fd_));
        }

        bool setNonblocking(void) const noexcept
        {
            auto ret = fcntl(fd_, F_GETFL);
            if (ret == -1)
                return false;
            return fcntl(fd_, F_SETFL, ret | O_NONBLOCK) != -1;
        }

        bool operator!(void) const noexcept
        {
            return fd_ == -1;
        }

        explicit operator bool(void) const noexcept
        {
            return fd_ != -1;
        }

        FileDescriptor & operator=(const FileDescriptor & other) noexcept
        {
            if (this != &other)
                fd_ = other.fd_ == -1 ? -1 : ::dup(other.fd_);
            return *this;
        }

        FileDescriptor & operator=(FileDescriptor && other) noexcept
        {
            if (this != &other)
            {
                fd_ = other.fd_;
                other.fd_ = -1;
            }
            return *this;
        }

        bool operator<(const FileDescriptor & other) const noexcept
        {
            return fd_ < other.fd_;
        }

    private:
        int fd_{ -1 };
    };

}