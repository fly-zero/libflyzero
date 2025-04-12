#pragma once

#include <fcntl.h>
#include <unistd.h>

#include <utility>

namespace flyzero {

class file_descriptor {
public:
    /**
     * @brief 默认构造函数
     */
    constexpr file_descriptor() = default;

    /**
     * @brief 构造函数
     * @param fd 文件描述符
     */
    explicit file_descriptor(int fd) noexcept;

    /**
     * @brief 复制构造函数
     */
    file_descriptor(const file_descriptor& other) noexcept;

    /**
     * @brief 移动构造函数
     */
    file_descriptor(file_descriptor&& other) noexcept;

    /**
     * @brief 析构函数
     */
    ~file_descriptor() noexcept;

    /**
     * @brief 获取文件描述符
     */
    int get() const noexcept;

    /**
     * @brief 关闭文件描述符
     */
    void close() noexcept;

    /**
     * @brief 释放文件描述符的所有权
     * @return int 文件描述符
     */
    int release() noexcept;

    /**
     * @brief 复制文件描述符
     * @return file_descriptor
     * 成功时，返回新的文件描述符；失败时，返回无效的文件描述符
     */
    file_descriptor clone() const noexcept;

    /**
     * @brief 设置文件描述符为非阻塞
     * @return 成功时，返回 true；失败时，返回 false
     */
    bool set_nonblocking() const noexcept;

    /**
     * @brief 转换为 bool
     */
    explicit operator bool() const noexcept;

    /**
     * @brief 复制赋值
     */
    file_descriptor& operator=(const file_descriptor& other) noexcept;

    /**
     * @brief 移动赋值
     */
    file_descriptor& operator=(file_descriptor&& other) noexcept;

    /**
     * @brief 比较运算符
     */
    bool operator<(const file_descriptor& other) const noexcept;

private:
    int fd_{-1};  ///< 文件描述符
};

inline file_descriptor::file_descriptor(int const fd) noexcept : fd_(fd) {}

inline file_descriptor::file_descriptor(const file_descriptor& other) noexcept
    : fd_{other.fd_ == -1 ? -1 : ::fcntl(other.fd_, F_DUPFD, 0)} {}

inline file_descriptor::file_descriptor(file_descriptor&& other) noexcept
    : fd_{std::exchange(other.fd_, -1)} {}

inline file_descriptor::~file_descriptor() noexcept {
    if (fd_ >= 0) {
        ::close(fd_);
    }
}

inline int file_descriptor::get() const noexcept { return fd_; }

inline void file_descriptor::close() noexcept {
    ::close(fd_);
    fd_ = -1;
}

inline int file_descriptor::release() noexcept { return std::exchange(fd_, -1); }

inline file_descriptor file_descriptor::clone() const noexcept {
    if (fd_ == -1) return file_descriptor();
    return file_descriptor(::fcntl(fd_, F_DUPFD, 0));
}

inline bool file_descriptor::set_nonblocking() const noexcept {
    if (fd_ < 0) return false;
    auto const ret = ::fcntl(fd_, F_GETFL);
    if (ret < 0) return false;
    return ::fcntl(fd_, F_SETFL, ret | O_NONBLOCK) == 0;
}

inline file_descriptor::operator bool(void) const noexcept { return fd_ != -1; }

inline file_descriptor& file_descriptor::operator=(const file_descriptor& other) noexcept {
    if (this != &other) [[likely]] {
        *this = other.clone();
    }
    return *this;
}

inline file_descriptor& file_descriptor::operator=(file_descriptor&& other) noexcept {
    if (this != &other) [[likely]] {
        if (fd_ >= 0) {
            ::close(fd_);
        }
        fd_ = std::exchange(other.fd_, -1);
    }
    return *this;
}

inline bool file_descriptor::operator<(const file_descriptor& other) const noexcept {
    return fd_ < other.fd_;
}

}  // namespace flyzero
