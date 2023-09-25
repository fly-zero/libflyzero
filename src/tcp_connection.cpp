#include "tcp_connection.h"

#include <sys/socket.h>

#include "circular_buffer.h"

namespace flyzero {

void TcpConnection::on_read() {
    while (true) {
        // 获取可写入的空间
        auto const wbuf = circular_buffer_get_writable(read_cb_.get());
        if (wbuf.size > 0) [[likely]] {
            // 有可写入的空间，读取数据
            auto const n = ::recv(fd(), wbuf.data, wbuf.size, 0);
            if (n > 0) [[likely]] {
                circular_buffer_push_data(read_cb_.get(), n);
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

inline size_t TcpConnection::consume() {
    // 消费可读数据
    auto const rbuf = circular_buffer_get_readable(read_cb_.get());
    auto const consume_size = on_read(rbuf.data, rbuf.size);
    if (consume_size > 0) [[likely]]
        circular_buffer_pop_data(read_cb_.get(), consume_size);
    return consume_size;
}

}  // namespace flyzero