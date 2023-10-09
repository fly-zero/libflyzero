#include <lru_cache.h>

#include <chrono>
#include <iostream>
#include <thread>

struct Hash {
    std::size_t operator()(const std::string &key) const {
        return boost::hash_range(key.begin(), key.end());
    }

    std::size_t operator()(const char *key) const {
        return boost::hash_range(key, key + std::strlen(key));
    }
};

struct Equal {
    bool operator()(const std::string &lhs, const std::string &rhs) const {
        return lhs == rhs;
    }

    bool operator()(const std::string &lhs, const char *rhs) const {
        return lhs == rhs;
    }

    bool operator()(const char *lhs, const std::string &rhs) const {
        return lhs == rhs;
    }
};

static void test_bucket_expand() {
    flyzero::LruCache<std::string, int, Hash, Equal> cache{
        std::chrono::seconds{10}};

    // 获取初始桶数
    auto const init_bucket_count = cache.bucket_count();

    // 插入与桶数相同的元素
    for (int i = 0; i <= init_bucket_count; ++i) {
        cache.insert(std::chrono::steady_clock::now(), std::to_string(i), i);
    }

    // 获取扩容后的桶数
    auto const bucket_count = cache.bucket_count();
    assert(bucket_count > init_bucket_count);

    // 查找所有元素
    for (int i = 0; i <= init_bucket_count; ++i) {
        assert(cache.find(std::to_string(i)) != cache.end());
    }
}

static void test_insert_find_touch() {
    flyzero::LruCache<std::string, int, Hash, Equal> cache{
        std::chrono::seconds{10}};

    // 测试插入
    auto const [it, inserted] =
        cache.insert(std::chrono::steady_clock::now(), "hello", 1);
    assert(inserted);

    // 测试异构查询
    assert(cache.find("hello") == it);

    // 测试更新
    cache.touch(std::chrono::steady_clock::now(), it);
}

static void test_expired() {
    flyzero::LruCache<std::string, int, Hash, Equal> cache{
        std::chrono::seconds{1}};

    for (auto i = 0; i < 5; ++i) {
        cache.insert(std::chrono::steady_clock::now(), std::to_string(i), i);
        std::this_thread::sleep_for(std::chrono::milliseconds{300});
    }

    int next_expected = 0;
    auto const n =
        cache.clear_expired(std::chrono::steady_clock::now(),
                            [&next_expected](const std::string &, int v) {
                                assert(v == next_expected++);
                            });

    assert(n == 2);
}

int main() {
    test_bucket_expand();
    test_insert_find_touch();
    test_expired();
}