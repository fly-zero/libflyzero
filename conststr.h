#pragma once
#include <stdexcept>

namespace flyzero
{
    class conststr
    {
        const char* p;
        std::size_t sz;

    public:
        template<std::size_t N>
        explicit constexpr conststr(const char(&a)[N]) : p(a), sz(N - 1) { }

        // constexpr functions signal errors by throwing exceptions
        // in C++11, they must do so from the conditional operator ?:
        constexpr char operator[](std::size_t n) const
        {
            return n < sz ? p[n] : throw std::out_of_range("");
        }

        constexpr std::size_t size() const { return sz; }
    };
}

