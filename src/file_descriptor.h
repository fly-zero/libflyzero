#pragma once

#include <unistd.h>
#include <fcntl.h>

namespace flyzero
{

    class file_descriptor
    {
    public:
        /**
         * @brief Default constructor
         */ 
        constexpr file_descriptor(void) = default;

        /**
         * @brief Construct from fd number
         * 
         * @param fd File decriptor number
         */
        explicit file_descriptor(int const fd) noexcept
            : fd_(fd)
        {
        }

        /**
         * @brief Copy constructor
         */
        file_descriptor(const file_descriptor & other) noexcept
            : fd_(other.fd_ == -1 ? -1 : ::fcntl(other.fd_, F_DUPFD, 0))
        {
        }

        /**
         * @brief Move constructor
         */
        file_descriptor(file_descriptor && other) noexcept
            : fd_(other.fd_)
        {
            other.fd_ = -1;
        }

        /**
         * @brief Destructor
         */
        ~file_descriptor(void) noexcept { if (fd_ >= 0) ::close(fd_); }

        /**
         * @brief Get the file descriptor number
         */
        int get(void) const noexcept { return fd_; }

        /**
         * @brief Close file descriptor
         */
        void close(void) noexcept
        {
            ::close(fd_);
            fd_ = -1;
        }

        /**
         * @brief Release the ownership of file descriptor
         * 
         * @return int Return the file descriptor number
         */
        int release(void) noexcept
        {
            auto ret = fd_;
            fd_ = -1;
            return ret;
        }

        /**
         * @brief Clone the file descriptor
         * 
         * @return FileDescriptor On success, return a new file descriptor
         *                        On error, return a empty file descriptor 
         */
        file_descriptor clone(void) const noexcept { return file_descriptor(::fcntl(fd_, F_DUPFD, 0)); }

        /**
         * @brief Set the file descriptor nonblocking
         * 
         * @return true Success
         * @return false Failed
         */
        bool set_nonblocking(void) const noexcept
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
        file_descriptor & operator=(const file_descriptor & other) noexcept
        {
            if (this != &other)
                fd_ = other.fd_ == -1 ? -1 : ::fcntl(other.fd_, F_DUPFD, 0);
            return *this;
        }

        /**
         * @brief Move assignment
         */
        file_descriptor & operator=(file_descriptor && other) noexcept
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
        bool operator<(const file_descriptor & other) const noexcept { return fd_ < other.fd_; }

    private:
        int fd_{ -1 };
    };

}
