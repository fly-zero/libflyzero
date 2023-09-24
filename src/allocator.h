#pragma once

#include <functional>

namespace flyzero
{

    template <class Type>
    class allocator
    {
    public:
        using value_type = Type;
        using pointer = value_type *;
        using const_pointer = value_type const *;
        using reference = value_type &;
        using const_reference = const value_type &;
        using alloc_type = std::function<void*(std::size_t)>;
        using dealloc_type = std::function<void(void *)>;

        template <class OtherType>
        struct rebind
        {
            using other = allocator<OtherType>;
        };

        allocator() = default;

        allocator(const alloc_type & alloc, const dealloc_type & dealloc)
            : alloc_(alloc)
            , dealloc_(dealloc)
        {
        }

        explicit allocator(const allocator &) = default;

        template <class OtherType>
        explicit allocator(const allocator<OtherType> & other)
            : alloc_(other.get_alloc())
            , dealloc_(other.get_dealloc())
        {
        }

        pointer allocate(std::size_t const n)
        {
            return static_cast<pointer>(alloc_(n * sizeof (value_type)));
        }

        void deallocate(const pointer p, std::size_t n)
        {
            dealloc_(p);
        }

        static void construct(pointer const p, const_reference val)
        {
            ::new (static_cast<void *>(p)) Type(val);
        }

        template <class U, class... Args>
        static void construct(U * p, Args&&... args)
        {
            ::new (static_cast<void *>(p)) U(std::forward<Args>(args)...);
        }

        static void destroy(pointer p)
        {
            p->~Type();
        }

        template <class U>
        static void destroy(U * p)
        {
            p->~U();
        }

        const alloc_type & get_alloc() const
        {
            return alloc_;
        }

        const dealloc_type & get_dealloc() const
        {
            return dealloc_;
        }

        static pointer address(reference x)
        {
            return &x;
        }

        static const_pointer address(const_reference x)
        {
            return &x;
        }

        bool operator==(const_reference other) const
        {
            return alloc_ == other.alloc_ && dealloc_ == other.dealloc_;
        }

        bool operator!=(const_reference other) const
        {
            return !this->operator==(other);
        }

        static std::size_t max_size()
        {
            return std::size_t(-1) / sizeof (value_type);
        }

    private:
        alloc_type alloc_{ malloc };
        dealloc_type dealloc_{ free };
    };

}
