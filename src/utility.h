#pragma once

#include <cstdint>

namespace flyzero {
namespace utility {

inline unsigned long long next_pow2(unsigned long long n) {
    // 利用 GCC 内联函数 __builtin_clzll 求出 n - 1 的二进制表示中最高位 1 的位置
    return n == 0 ? 1 : 1ULL << (64 - __builtin_clzll(n - 1));
}

inline uint64_t reverse_byte_order(const uint64_t val) {
    return __builtin_bswap64(val);
}

inline uint32_t reverse_byte_order(const uint32_t val) {
    return __builtin_bswap32(val);
}

}  // namespace utility
}  // namespace flyzero
