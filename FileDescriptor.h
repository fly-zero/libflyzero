#pragma once

#include <unistd.h>
#include <fcntl.h>

namespace flyzero
{

    class FileDescriptor
    {
    public:
        /**
         * @brief Default constructor
         */ 
        constexpr FileDescriptor(void) = default;

        /**
         * @brief Construct from fd number
         * 
         * @param fd File decriptor number
         */
        explicit FileDescriptor(int const fd) noexcept
            : fd_(fd)
        {
        }

        /**
         * @brief Copy constructor
         */
        FileDescriptor(const FileDescriptor & other) noexcept
            : fd_(other.fd_ == -1 ? -1 : ::fcntl(other.fd_, F_DUPFD, 0))
        {
        }

        /**
         * @brief Move constructor
         */
        FileDescriptor(FileDescriptor && other) noexcept
            : fd_(other.fd_)
        {
            other.fd_ = -1;
        }

        /**
         * @brief Destructor
         */
        ~FileDescriptor(void) noexcept { if (fd_ >= 0) ::close(fd_); }

        /**
         * @brief Get the file descriptor number
         */
        int Get(void) const noexcept { return fd_; }

        /**
         * @brief Close file descriptor
         */
        void Close(void) noexcept
        {
            ::close(fd_);
            fd_ = -1;
        }

        /**
         * @brief Clone the file descriptor
         * 
         * @return FileDescriptor On success, return a new file descriptor
         *                        On error, return a empty file descriptor 
         */
        FileDescriptor Clone(void) const noexcept { return FileDescriptor(::fcntl(fd_, F_DUPFD, 0)); }

        /**
         * @brief Set the file descriptor nonblocking
         * 
         * @return true Success
         * @return false Failed
         */
        bool SetNonblocking(void) const noexcept
        {
            auto const ret = ::fcntl(fd_, F_GETFL);
            if (ret == -1)
                return false;
            return ::fcntl(fd_, F_SETFL, ret | O_NONBLOCK) != -1;
        }

        /**
         * @brief Return true, if fd_ is a valid file descriptor value; otherwise return false
         */
        explicit operator bool(void) const noexcept { return fd_ != -1; }

        /**
         * @brief Copy assignment
         */
        FileDescriptor & operator=(const FileDescriptor & other) noexcept
        {
            if (this != &other)
                fd_ = other.fd_ == -1 ? -1 : ::fcntl(other.fd_, F_DUPFD, 0);
            return *this;
        }

        /**
         * @brief Move assignment
         */
        FileDescriptor & operator=(FileDescriptor && other) noexcept
        {
            if (this != &other)
            {
                fd_ = other.fd_;
                other.fd_ = -1;
            }
            return *this;
        }

        /**
         * @brief Operator < overload
         */
        bool operator<(const FileDescriptor & other) const noexcept { return fd_ < other.fd_; }

    private:
        int fd_{ -1 };
    };

}
