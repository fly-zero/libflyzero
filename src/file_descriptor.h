#pragma once

#include <fcntl.h>
#include <unistd.h>

#include <utility>

namespace flyzero {

class FileDescriptor {
public:
    /**
     * @brief 默认构造函数
     */
    constexpr FileDescriptor() = default;

    /**
     * @brief 构造函数
     * @param fd 文件描述符
     */
    explicit FileDescriptor(int fd) noexcept;

    /**
     * @brief 复制构造函数
     */
    FileDescriptor(const FileDescriptor& other) noexcept;

    /**
     * @brief 移动构造函数
     */
    FileDescriptor(FileDescriptor&& other) noexcept;

    /**
     * @brief 析构函数
     */
    ~FileDescriptor() noexcept;

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
     * @return FileDescriptor
     * 成功时，返回新的文件描述符；失败时，返回无效的文件描述符
     */
    FileDescriptor clone() const noexcept;

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
    FileDescriptor& operator=(const FileDescriptor& other) noexcept;

    /**
     * @brief 移动赋值
     */
    FileDescriptor& operator=(FileDescriptor&& other) noexcept;

    /**
     * @brief 比较运算符
     */
    bool operator<(const FileDescriptor& other) const noexcept;

private:
    int fd_{-1};  ///< 文件描述符
};

inline FileDescriptor::FileDescriptor(int const fd) noexcept : fd_(fd) {}

inline FileDescriptor::FileDescriptor(const FileDescriptor& other) noexcept
    : fd_{other.fd_ == -1 ? -1 : ::fcntl(other.fd_, F_DUPFD, 0)} {}

inline FileDescriptor::FileDescriptor(FileDescriptor&& other) noexcept
    : fd_{std::exchange(other.fd_, -1)} {}

inline FileDescriptor::~FileDescriptor() noexcept {
    if (fd_ >= 0) {
        ::close(fd_);
    }
}

inline int FileDescriptor::get() const noexcept { return fd_; }

inline void FileDescriptor::close() noexcept {
    ::close(fd_);
    fd_ = -1;
}

inline int FileDescriptor::release() noexcept { return std::exchange(fd_, -1); }

inline FileDescriptor FileDescriptor::clone() const noexcept {
    if (fd_ == -1) return FileDescriptor();
    return FileDescriptor(::fcntl(fd_, F_DUPFD, 0));
}

inline bool FileDescriptor::set_nonblocking() const noexcept {
    if (fd_ < 0) return false;
    auto const ret = ::fcntl(fd_, F_GETFL);
    if (ret < 0) return false;
    return ::fcntl(fd_, F_SETFL, ret | O_NONBLOCK) == 0;
}

inline FileDescriptor::operator bool(void) const noexcept { return fd_ != -1; }

inline FileDescriptor& FileDescriptor::operator=(
    const FileDescriptor& other) noexcept {
    if (this != &other) [[likely]] {
        *this = other.clone();
    }
    return *this;
}

inline FileDescriptor& FileDescriptor::operator=(
    FileDescriptor&& other) noexcept {
    if (this != &other) [[likely]] {
        if (fd_ >= 0) {
            ::close(fd_);
        }
        fd_ = std::exchange(other.fd_, -1);
    }
    return *this;
}

inline bool FileDescriptor::operator<(
    const FileDescriptor& other) const noexcept {
    return fd_ < other.fd_;
}

}  // namespace flyzero
