#pragma once

#include <memory>

#include "circular_buffer.h"
#include "event_dispatch.h"
#include "file_descriptor.h"

namespace flyzero {

class tcp_connection : public event_dispatch::io_listener {
    struct deleter {
        void operator()(circular_buffer *cb) const noexcept;
    };

    using cb = std::unique_ptr<circular_buffer, deleter>;

public:
    /**
     * @brief 构造函数
     *
     * @param sock 套接字
     * @param rcb_size 读环形缓冲区大小
     * @param wcb_size 写环形缓冲区大小
     */
    tcp_connection(int sock, size_t rcb_size, size_t wcb_size);

    /**
     * @brief 构造函数
     *
     * @param sock 套接字
     * @param rcb_size 读环形缓冲区大小
     * @param wcb_size 写环形缓冲区大小
     */
    tcp_connection(file_descriptor &&sock, size_t rcb_size, size_t wcb_size);

    /**
     * @brief 禁止拷贝
     */
    tcp_connection(const tcp_connection &) = delete;

    /**
     * @brief 移动构造函数
     */
    tcp_connection(tcp_connection &&) = default;

    /**
     * @brief 析构函数
     */
    ~tcp_connection() override = default;

    /**
     * @brief 禁止拷贝
     */
    void operator=(const tcp_connection &) = delete;

    /**
     * @brief 移动赋值
     */
    tcp_connection &operator=(tcp_connection &&) = default;

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

    /**
     * @brief 生产可写数据
     */
    size_t produce();

    /**
     * @brief 创建环形缓冲区
     *
     * @param size 缓冲区大小，若为 0，则返回空的缓冲区
     * @return 环形缓冲区对象指针
     */
    static cb create_cb(size_t size);

protected:
    /**
     * @brief 读取数据处理函数
     */
    virtual size_t on_read(const void *data, size_t size) = 0;

    /**
     * @brief 写数据处理函数
     */
    virtual size_t on_write(void *data, size_t size) = 0;

    /**
     * @brief 关闭连接处理函数
     */
    virtual void on_close() = 0;

private:
    cb rcb_{};  ///< 读环形缓冲区对象指针
    cb wcb_{};  ///< 写环形缓冲区对象指针
};

inline void tcp_connection::deleter::operator()(circular_buffer *cb) const noexcept {
    circular_buffer_destroy(cb);
}

inline tcp_connection::tcp_connection(int sock, size_t rcb_size, size_t wcb_size)
    : tcp_connection{file_descriptor(sock), rcb_size, wcb_size} {}

inline tcp_connection::tcp_connection(file_descriptor &&sock, size_t rcb_size, size_t wcb_size)
    : event_dispatch::io_listener{std::move(sock)},
      rcb_{create_cb(rcb_size)},
      wcb_{create_cb(wcb_size)} {}

}  // namespace flyzero
