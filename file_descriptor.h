#pragma once

#include <unistd.h>
#include <fcntl.h>

namespace flyzero
{

    class file_descriptor
    {
    public:
        constexpr file_descriptor() = default;

        explicit file_descriptor(int const fd) noexcept
            : fd_(fd)
        {
        }

        file_descriptor(const file_descriptor & other) noexcept
            : fd_(other.fd_ == -1 ? -1 : ::fcntl(other.fd_, F_DUPFD, 0))
        {
        }

        file_descriptor(file_descriptor && other) noexcept
            : fd_(other.fd_)
        {
            other.fd_ = -1;
        }

        ~file_descriptor() noexcept { if (fd_ >= 0) ::close(fd_); }

        int get() const noexcept { return fd_; }

        void close() noexcept
        {
            ::close(fd_);
            fd_ = -1;
        }

        file_descriptor clone() const noexcept { return file_descriptor(::fcntl(fd_, F_DUPFD, 0)); }

        bool set_nonblocking() const noexcept
        {
            auto const ret = fcntl(fd_, F_GETFL);
            if (ret == -1)
                return false;
            return fcntl(fd_, F_SETFL, ret | O_NONBLOCK) != -1;
        }

        bool operator!() const noexcept { return fd_ == -1; }

        explicit operator bool() const noexcept { return fd_ != -1; }

        file_descriptor & operator=(const file_descriptor & other) noexcept
        {
            if (this != &other)
                fd_ = other.fd_ == -1 ? -1 : ::fcntl(other.fd_, F_DUPFD, 0);
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

        bool operator<(const file_descriptor & other) const noexcept { return fd_ < other.fd_; }

        size_t write(const char * buff, size_t size) const;

    private:
        int fd_{ -1 };
    };

}
