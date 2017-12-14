#pragma once

#include <utility>
#include <type_traits>

namespace flyzero
{

    template <class Type, class Alloc, class Dealloc>
    class allocator
    {
    public:
        using value_type = Type;
        using pointer = value_type *;
        using const_pointer = const pointer;
        using reference = value_type &;
        using const_reference = const reference;
        using alloc_type = Alloc;
        using dealloc_type = Dealloc;
        using alloc_param_type = typename std::conditional<std::is_fundamental<alloc_type>::value, alloc_type, const alloc_type &>::type;
        using dealloc_param_type = typename std::conditional<std::is_fundamental<dealloc_type>::value, dealloc_type, const dealloc_type &>::type;

        template <class OtherType, class OtherAlloc = Alloc, class OtherDealloc = Dealloc>
        struct rebind
        {
            using other = allocator<OtherType, OtherAlloc, OtherDealloc>;
        };

        allocator() = default;

        allocator(alloc_param_type alloc, dealloc_param_type dealloc)
            : alloc_(alloc)
            , dealloc_(dealloc)
        {
        }

        explicit allocator(const allocator &) = default;

        template <class OtherType, class OtherAlloc, class OtherDealloc>
        explicit allocator(const allocator<OtherType, OtherAlloc, OtherDealloc> & other)
            : alloc_(other.get_alloc())
            , dealloc_(other.get_dealloc())
        {
        }

        pointer allocate(std::size_t const n)
        {
            return static_cast<pointer>(alloc_(n * sizeof (value_type)));
        }

        void deallocate(pointer p, std::size_t n)
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
            p->~_Type();
        }

        template <class U>
        static void destroy(U * p)
        {
            p->~U();
        }

        alloc_param_type get_alloc() const
        {
            return alloc_;
        }

        dealloc_param_type get_dealloc() const
        {
            return dealloc_;
        }

    private:
        alloc_type alloc_;
        dealloc_type dealloc_;
    };

}
