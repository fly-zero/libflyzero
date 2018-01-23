#pragma once

#include <memory>
#include <cassert>

#include "allocator.h"

namespace flyzero
{
    
    template <typename Type, typename Alloc = flyzero::allocator<Type> >
    class circular_queue
    {
        using allocator_type = Alloc;

        class deleter_type
        {
        public:
            explicit deleter_type(circular_queue & cq)
                : cq_(cq)
            {
            }

            void operator()(Type * p) const
            {
                auto & alloc = cq_.get_alloc();
                for (std::size_t i = 0; i < cq_.get_capacity(); ++i)
                    alloc.destroy(p + i);
                alloc.deallocate(p, 0);
            }

        private:
            circular_queue & cq_;
        };

        using pointer = std::unique_ptr<Type[], deleter_type>;

    public:
        explicit circular_queue(std::size_t capacity, const allocator_type & alloc = allocator_type())
            : alloc_(alloc)
            , capacity_(capacity)
            , buffer_(initalize(capacity), deleter_type(*this))
            , ridx_(0)
            , widx_(0)
        {
        }

        circular_queue(const circular_queue &) = delete;

        circular_queue(circular_queue && other) noexcept
            : alloc_(std::move(other.alloc_))
            , buffer_(std::move(other.buffer_))
        {
            std::swap(capacity_, other.capacity_);
            std::swap(ridx_, other.ridx_);
            std::swap(widx_, other.widx_);
        }

        circular_queue & operator=(const circular_queue &) = delete;

        circular_queue & operator=(circular_queue && other) noexcept
        {
            if (this != &other)
            {
                alloc_ = other.alloc_;

                capacity_ = other.capacity_;
                other.capacity_ = 0;

                buffer_ = std::move(other.buffer_);

                widx_ = other.widx_;
                other.widx_ = 0;

                ridx_ = other.ridx_;
                other.ridx_ = 0;
            }

            return *this;
        }

        allocator_type & get_alloc(void) { return alloc_; }

        std::size_t get_capacity(void) const { return capacity_; }

        std::size_t size(void) const
        {
            return widx_ - ridx_;
        }

        bool full(void) const
        {
            return size() == capacity_;
        }

        bool empty(void) const
        {
            return size() == 0;
        }

        template <typename...Args>
        bool push(Args&&...args)
        {
            if (full())
                return false;

            buffer_[widx_ % capacity_] = Type(std::forward<Args>(args)...);
            ++widx_;
            return true;
        }

        void pop(void)
        {
            if (empty())
                return;

            alloc_.destroy(&buffer_[ridx_ % capacity_]);
            ++ridx_;
        }

        typename std::conditional<std::is_fundamental<Type>::value, Type, Type &>::type front(void)
        {
            assert(!empty());
            return buffer_[ridx_ % capacity_];
        }

        typename std::conditional<std::is_fundamental<Type>::value, Type, const Type &>::type front(void) const
        {
            assert(!empty());
            return buffer_[ridx_ % capacity_];
        }

    protected:
        Type * initalize(std::size_t capacity)
        {
            auto ret = alloc_.allocate(capacity * sizeof (Type));
            for (std::size_t i = 0; i < capacity; ++i)
                alloc_.construct(ret + i);
            return ret;
        }

    private:
        allocator_type alloc_;
        std::size_t capacity_;
        pointer buffer_;
        std::size_t ridx_;
        std::size_t widx_;
    };

}
