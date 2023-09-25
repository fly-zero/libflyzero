#pragma once

#include <memory>

#include "circular_buffer.h"
#include "event_dispatch.h"
#include "file_descriptor.h"

namespace flyzero {

class TcpConnection : public EventDispatch::IoListener {
    struct Deleter {
        void operator()(circular_buffer *cb) const noexcept;
    };

    using CircularBufferPtr = std::unique_ptr<circular_buffer, Deleter>;

public:
    /**
     * @brief 构造函数
     * @param sock 套接字
     */
    explicit TcpConnection(int);

    /**
     * @brief 构造函数
     * @param sock 套接字
     */
    explicit TcpConnection(FileDescriptor &&sock) noexcept;

    /**
     * @brief 禁止拷贝
     */
    TcpConnection(const TcpConnection &) = delete;

    /**
     * @brief 移动构造函数
     */
    TcpConnection(TcpConnection &&) = default;

    /**
     * @brief 析构函数
     */
    ~TcpConnection() override = default;

    /**
     * @brief 禁止拷贝
     */
    void operator=(const TcpConnection &) = delete;

    /**
     * @brief 移动赋值
     */
    TcpConnection &operator=(TcpConnection &&) = default;

private:
    /**
     * @brief 读取数据
     */
    void on_read() override final;

    /**
     * @brief 写入数据
     */
    void on_write() override final;

    /**
     * @brief 消费可读数据
     */
    size_t consume();

protected:
    /**
     * @brief 读取数据处理函数
     */
    virtual size_t on_read(const void *data, size_t size) = 0;

    /**
     * @brief 关闭连接处理函数
     */
    virtual void on_close() = 0;

private:
    CircularBufferPtr read_cb_{};  ///< 环形缓冲区对象指针
};

inline void TcpConnection::Deleter::operator()(
    circular_buffer *cb) const noexcept {
    circular_buffer_destroy(cb);
}

inline TcpConnection::TcpConnection(int sock)
    : TcpConnection{FileDescriptor(sock)} {}

inline TcpConnection::TcpConnection(FileDescriptor &&sock) noexcept
    : EventDispatch::IoListener{std::move(sock)} {}

}  // namespace flyzero
