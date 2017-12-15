#pragma once
#include <stdexcept>

namespace flyzero
{
    class conststr
    {
    public:
        template<std::size_t N>
        explicit constexpr conststr(const char(&a)[N]) : str_(a), length_(N - 1) { }

        // constexpr functions signal errors by throwing exceptions
        // in C++11, they must do so from the conditional operator ?:
        constexpr char operator[](const std::size_t n) const
        {
            return n < length_ ? str_[n] : throw std::out_of_range("");
        }

        constexpr std::size_t size() const { return length_; }

        const char * begin() const { return str_; }

        const char * end() const { return str_ + length_; }

    private:
        const char* str_;
        std::size_t length_;
    };
}

