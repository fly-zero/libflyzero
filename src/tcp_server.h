#pragma once

#include <arpa/inet.h>
#include <netinet/in.h>

#include <cassert>
#include <functional>
#include <memory>

#include "event_dispatch.h"
#include "file_descriptor.h"

namespace flyzero {

class TcpServer : public EventDispatch::IoListener {
public:
    /**
     * @brief 构造函数
     * @param sock 套接字
     */
    TcpServer(int sock);

    /**
     * @brief 禁止拷贝
     */
    TcpServer(const TcpServer &) = delete;

    /**
     * @brief 移动构造函数
     */
    TcpServer(TcpServer &&) = default;

    /**
     * @brief 析构函数
     */
    ~TcpServer(void) override = default;

    /**
     * @brief 禁止拷贝
     */
    void operator=(const TcpServer &) = delete;

    /**
     * @brief 移动赋值
     */
    TcpServer &operator=(TcpServer &&) = default;

protected:
    /**
     * @brief 接受连接
     * @param sock 套接字
     * @param addr 地址
     * @param addrlen 地址长度
     */
    virtual void on_accept(file_descriptor &&sock, const sockaddr_storage &addr,
                           socklen_t addrlen) = 0;

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

inline TcpServer::TcpServer(int sock) : IoListener(sock) {}

}  // namespace flyzero
