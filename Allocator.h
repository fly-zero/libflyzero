#pragma once

#include <new>
#include <utility>
#include <type_traits>

namespace flyzero
{

	template <class _Type, class _Alloc, class _Dealloc>
	class Allocator
	{
    public:
        using value_type = _Type;
        using pointer = value_type *;
        using const_pointer = const pointer;
        using reference = value_type &;
        using const_reference = const reference;
        using alloc_type = _Alloc;
        using dealloc_type = _Dealloc;
        using alloc_param_type = typename std::conditional<std::is_fundamental<alloc_type>::value, alloc_type, const alloc_type &>::type;
        using dealloc_param_type = typename std::conditional<std::is_fundamental<dealloc_type>::value, dealloc_type, const dealloc_type &>::type;

        template <class _OtherType, class _OtherAlloc = _Alloc, class _OtherDealloc = _Dealloc>
        struct rebind
        {
            using other = Allocator<_OtherType, _OtherAlloc, _OtherDealloc>;
        };

        Allocator(void) = default;

        Allocator(alloc_param_type alloc, dealloc_param_type dealloc)
            : alloc_(alloc)
            , dealloc_(dealloc)
        {
        }

        explicit Allocator(const Allocator &) = default;

        template <class _OtherType, class _OtherAlloc, class _OtherDealloc>
        explicit Allocator(const Allocator<_OtherType, _OtherAlloc, _OtherDealloc> & other)
            : alloc_(other.getAlloc())
            , dealloc_(other.getDealloc())
        {
        }

		pointer allocate(std::size_t n)
		{
			return static_cast<pointer>(alloc_(n * sizeof (value_type)));
		}

		void deallocate(pointer p, std::size_t n)
		{
			dealloc_(p);
		}

		static void construct(pointer p, const_reference val)
		{
			::new (static_cast<void *>(p)) _Type(val);
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

        alloc_param_type getAlloc(void) const
        {
            return alloc_;
        }

        dealloc_param_type getDealloc(void) const
        {
            return dealloc_;
        }

    private:
        alloc_type alloc_;
        dealloc_type dealloc_;
	};

}
