#pragma once

#include <unistd.h>
#include <fcntl.h>

namespace flyzero
{

    class file_descriptor
    {
    public:
        constexpr file_descriptor(void) = default;

        explicit file_descriptor(int const fd) noexcept
            : fd_(fd)
        {
        }

        file_descriptor(const file_descriptor & other) noexcept
            : fd_(other.fd_ == -1 ? -1 : ::dup(other.fd_))
        {
        }

        file_descriptor(file_descriptor && other) noexcept
            : fd_(other.fd_)
        {
            other.fd_ = -1;
        }

        ~file_descriptor(void) noexcept
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

        file_descriptor clone(void) const noexcept
        {
            return file_descriptor(::dup(fd_));
        }

        bool set_nonblocking(void) const noexcept
        {
            auto const ret = fcntl(fd_, F_GETFL);
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

        file_descriptor & operator=(const file_descriptor & other) noexcept
        {
            if (this != &other)
                fd_ = other.fd_ == -1 ? -1 : ::dup(other.fd_);
            return *this;
        }

        file_descriptor & operator=(file_descriptor && other) noexcept
        {
            if (this != &other)
            {
                fd_ = other.fd_;
                other.fd_ = -1;
            }
            return *this;
        }

        bool operator<(const file_descriptor & other) const noexcept
        {
            return fd_ < other.fd_;
        }

        size_t write(const char * buff, size_t size) const;

    private:
        int fd_{ -1 };
    };

}
