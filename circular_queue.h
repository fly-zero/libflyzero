#pragma once

#include <memory>
#include <cassert>

#include "utility.h"

namespace flyzero
{

    template <typename Type>
    class circular_queue
    {
    public:
        using value_type = Type;
        using pointer = value_type *;
        using const_pointer = const value_type *;
        using allocator_type = std::function<void*(size_t)>;
        using deallocator_type = std::function<void(void *)>;

        explicit circular_queue(std::size_t const capacity, const allocator_type & allocator = malloc, const deallocator_type & deallocator = free)
            : capacity_(round_power2(capacity))
            , mask_(capacity_ - 1)
            , buffer_(malloc(capacity_ * sizeof (Type)))
            , ridx_(0)
            , widx_(0)
            , allocator_(allocator)
            , deallocator_(deallocator)
        {
        }

        circular_queue(const circular_queue &) = delete;

        circular_queue(circular_queue && other) noexcept
            : capacity_(other.capacity_)
            , mask_(other.mask_)
            , buffer_(other.buffer_)
            , ridx_(other.ridx_)
            , widx_(other.widx_)
            , allocator_(other.allocator_)
            , deallocator_(other.deallocator_)
        {
            other.capacity_ = 0;
            other.mask_ = 0;
            other.buffer_ = nullptr;
            other.ridx_ = 0;
            other.widx_ = 0;
        }

        ~circular_queue(void)
        {
            clear();
        }

        circular_queue & operator=(const circular_queue &) = delete;

        circular_queue & operator=(circular_queue && other) noexcept
        {
            if (this != &other)
            {
                clear();

                capacity_ = other.capacity_;
                other.capacity_ = 0;

                mask_ = other.mask_;
                other.mask_ = 0;

                buffer_ = other.buffer_;
                other.buffer_ = nullptr;

                ridx_ = other.ridx_;
                other.ridx_ = 0;

                widx_ = other.widx_;
                other.widx_ = 0;

                allocator_ = other.allocator_;
                deallocator_ = other.deallocator_;
            }

            return *this;
        }

        void clear(void)
        {
            if (!buffer_) return;
            while (!empty()) pop();
            deallocator_(buffer_);
            capacity_ = 0;
            mask_ = 0;
            buffer_ = nullptr;
        }

        std::size_t capacity(void) const { return capacity_; }

        std::size_t size(void) const { return widx_ - ridx_; }

        bool full(void) const { return size() == capacity_; }

        bool empty(void) const { return size() == 0; }

        template <typename...Args>
        bool push(Args&&...args)
        {
            assert(buffer_);
            return full()
                    ? false
                    : ((new(at(widx_++)) Type(std::forward<Args>(args)...)), true);
        }

        void pop(void)
        {
            assert(buffer_);
            if (!empty())
                at(ridx_++)->~Type();
        }

        typename std::conditional<std::is_fundamental<Type>::value, Type, Type &>::type front(void)
        {
            assert(!empty());
            return *at(ridx_);
        }

        typename std::conditional<std::is_fundamental<Type>::value, Type, const Type &>::type front(void) const
        {
            assert(!empty());
            return *at(ridx_);
        }

    protected:
        pointer at(size_t const idx) const
        {
            assert(buffer_);
            return static_cast<pointer *>(buffer_)[ idx & mask_ ];
        }

    private:
        std::size_t capacity_;
        std::size_t mask_;
        void * buffer_;
        std::size_t ridx_;
        std::size_t widx_;
        allocator_type allocator_;
        deallocator_type deallocator_;
    };

}
