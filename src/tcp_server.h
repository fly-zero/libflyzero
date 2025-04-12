#pragma once

#include <arpa/inet.h>
#include <netinet/in.h>

#include <cassert>
#include <functional>
#include <memory>

#include "event_dispatch.h"
#include "file_descriptor.h"

namespace flyzero {

class tcp_server : public event_dispatch::io_listener {
public:
    /**
     * @brief 构造函数
     * @param sock 套接字
     */
    tcp_server(int sock);

    /**
     * @brief 禁止拷贝
     */
    tcp_server(const tcp_server &) = delete;

    /**
     * @brief 移动构造函数
     */
    tcp_server(tcp_server &&) = default;

    /**
     * @brief 析构函数
     */
    ~tcp_server(void) override = default;

    /**
     * @brief 禁止拷贝
     */
    void operator=(const tcp_server &) = delete;

    /**
     * @brief 移动赋值
     */
    tcp_server &operator=(tcp_server &&) = default;

protected:
    /**
     * @brief 接受连接
     * @param sock 套接字
     * @param addr 地址
     * @param addrlen 地址长度
     */
    virtual void on_accept(file_descriptor        &&sock,
                           const sockaddr_storage &addr,
                           socklen_t               addrlen) = 0;

protected:
    /**
     * @brief 读取数据
     */
    void on_read(void) override final;

    /**
     * @brief 写入数据
     */
    void on_write(void) override final {}

    /**
     * @brief 监听指定地址和端口
     */
    static int listen(in_addr_t ip, uint16_t port);

    /**
     * @brief 监听指定 Unix 域套接字
     */
    static int listen(const char *unix_path);
};

inline tcp_server::tcp_server(int sock) : io_listener(sock) {}

}  // namespace flyzero
