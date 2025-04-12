#pragma once

#include <cstdint>

#include <tuple>
#include <string_view>
#include <system_error>

namespace flyzero {
namespace utility {

inline unsigned long long next_pow2(unsigned long long n) {
    // 利用 GCC 内联函数 __builtin_clzll 求出 n - 1 的二进制表示中最高位 1
    // 的位置
    return n == 0 ? 1 : 1ULL << (64 - __builtin_clzll(n - 1));
}

inline uint64_t reverse_byte_order(const uint64_t val) {
    return __builtin_bswap64(val);
}

inline uint32_t reverse_byte_order(const uint32_t val) {
    return __builtin_bswap32(val);
}

std::system_error system_error(int err, const char* format, ...)
    __attribute__((format(printf, 2, 3)));

/**
 * @brief 分割字符串
 *
 * @param str 被分割的字符串
 * @param delim 分隔符
 * @return std::pair<std::string_view, std::string_view> 分割后的字符串对
 * @note 如果没有找到分隔符，则返回 {str, ""}
 */
inline std::pair<std::string_view, std::string_view> split(std::string_view str, char delim) {
    auto pos = str.find(delim);
    if (pos == std::string_view::npos) return {str, {}};
    return {str.substr(0, pos), str.substr(pos + 1)};
}

}  // namespace utility
}  // namespace flyzero
