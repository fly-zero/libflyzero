#include <cassert>

#include <thread>

#include <circular_buffer.h>

// 测试单生产者单消费者
void test_sp_sc(size_t private_size) {
    // 创建环形缓冲区
    auto const cb = circular_buffer_create(nullptr, 1024, private_size, 0);
    assert(cb);

    // 获取私有数据
    if (private_size) {
        auto private_data = circular_buffer_get_private_data(cb);
        assert(private_data);
    }

    auto constexpr N = 10000000;
    volatile auto producer_done = false;

    // 创建生产者线程
    std::thread producer([cb] {
        for (int i = 0; i < N; ) {
            auto writable = circular_buffer_get_writable(cb);
            if (writable.size < sizeof(int)) {
                __builtin_ia32_pause();
                continue;
            }

            auto p = static_cast<int*>(writable.data);
            auto n = writable.size / sizeof(int);
            for (int j = 0; j < n; ++j) {
                p[j] = i++;
            }

            circular_buffer_push_data(cb, n * sizeof(int));
        }
    });

    // 创建消费者线程
    std::thread consumer([cb, &producer_done] {
        for (int i = 0; i < N; ) {
            auto readable = circular_buffer_get_readable(cb);
            if (readable.size < sizeof(int)) {
                if (producer_done) {
                    abort();
                }

                __builtin_ia32_pause();
                continue;
            }

            auto p = static_cast<int*>(readable.data);
            auto n = readable.size / sizeof(int);
            for (int j = 0; j < n; ++j) {
                assert(p[j] == i++);
            }

            circular_buffer_pop_data(cb, n * sizeof(int));
        }
    });

    producer.join();
    producer_done = true;
    consumer.join();

    // 释放环形缓冲区
    circular_buffer_destroy(cb);
}

void test_attach() {
    // 环形缓冲名名称
    auto const name = "test_attach";

    // 创建环形缓冲区
    auto const cb1 = circular_buffer_create(name, 1024, 0, 0);
    assert(cb1);

    // 附着环形缓冲区
    auto const cb2 = circular_buffer_attach(name);
    assert(cb2);

    // 分离环形缓冲区
    circular_buffer_detach(cb2);

    // 释放环形缓冲区
    circular_buffer_destroy(cb1);
}

int main() {
    test_sp_sc(0);
    test_sp_sc(100);
    test_attach();
}