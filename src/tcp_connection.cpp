#include "tcp_connection.h"

#include <sys/socket.h>

#include "circular_buffer.h"

namespace flyzero {

void tcp_connection::on_read() {
    while (true) {
        // 获取可写入的空间
        auto const wbuf = circular_buffer_get_writable(rcb_.get());
        if (wbuf.size > 0) [[likely]] {
            // 有可写入的空间，读取数据
            auto const n = ::recv(fd(), wbuf.data, wbuf.size, 0);
            if (n > 0) [[likely]] {
                circular_buffer_push_data(rcb_.get(), n);
                continue;
            } else if (n == 0) {
                // 处理剩余数据，连接关闭
                consume();
                on_close();
                return;
            } else if (errno == EAGAIN || errno == EWOULDBLOCK) [[likely]] {
                consume();
                return;
            } else {
                on_close();
                return;
            }
        } else if (consume() == 0) [[unlikely]] {
            // 没有可写入的空间，消费可读数据
            on_close();
            return;
        }
    }
}

void tcp_connection::on_write() {
    while (true) {
        auto const rbuf = circular_buffer_get_readable(wcb_.get());
        if (rbuf.size > 0) [[likely]] {
            auto const n = ::send(fd(), rbuf.data, rbuf.size, 0);
            if (n > 0) [[likely]] {
                circular_buffer_pop_data(wcb_.get(), n);
                continue;
            } else if (n == 0) {
                on_close();
                return;
            } else if (errno == EAGAIN || errno == EWOULDBLOCK) [[likely]] {
                return;
            } else {
                on_close();
                return;
            }
        } else if (produce() == 0) [[unlikely]] {
            on_close();
            return;
        }
    }
}

inline size_t tcp_connection::consume() {
    // 消费可读数据
    auto const rbuf         = circular_buffer_get_readable(rcb_.get());
    auto const consume_size = on_read(rbuf.data, rbuf.size);
    if (consume_size > 0) [[likely]]
        circular_buffer_pop_data(rcb_.get(), consume_size);
    return consume_size;
}

inline size_t tcp_connection::produce() {
    // 生产可写数据
    auto const wbuf         = circular_buffer_get_writable(wcb_.get());
    auto const produce_size = on_write(wbuf.data, wbuf.size);
    if (produce_size > 0) [[likely]]
        circular_buffer_push_data(wcb_.get(), produce_size);
    return produce_size;
}

auto tcp_connection::create_cb(size_t size) -> cb {
    if (size == 0) {
        return cb{nullptr};
    }

    auto const p = circular_buffer_create(nullptr, size, 0, 0);
    if (!p) {
        throw std::runtime_error{"Failed to create circular buffer"};
    }

    return cb{p};
}

}  // namespace flyzero