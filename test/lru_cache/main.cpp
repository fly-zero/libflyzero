#include <lru_cache.h>

#include <chrono>
#include <iostream>

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

int main() {
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

    return 0;
}