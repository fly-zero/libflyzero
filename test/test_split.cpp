#include <iostream>

#include "utility.h"

int main() {
    // 测试 split 函数
    struct {
        const char                                   *sample;    ///< 测试样例
        std::pair<std::string_view, std::string_view> expected;  ///< 期望的结果
    } const samples[] = {
        {"", {"", ""}},
        {"abc", {"abc", ""}},
        {":", {"", ""}},
        {"abc:", {"abc", ""}},
        {":abc", {"", "abc"}},
        {"a:b:c", {"a", "b:c"}},
        {"a:b:c:d", {"a", "b:c:d"}},
    };

    for (auto const &sample : samples) {
        auto const [first, second] = flyzero::utility::split(sample.sample, ':');
        if (first != sample.expected.first || second != sample.expected.second) {
            std::cerr << "Test failed for sample: " << sample.sample << std::endl;
            return 1;
        }
    }
}