#pragma once

#include <new>
#include <utility>

namespace flyzero
{
	template <class _Type, class _Alloc, class _Dealloc>
	class Allocator
	{
    public:
		typedef _Type value_type;
		typedef _Type * pointer;
		typedef const _Type * const_pointer;
		typedef _Type & reference;
		typedef const _Type & const_reference;

        Allocator(void) = default;

        Allocator(_Alloc alloc, _Dealloc dealloc)
            : alloc_(alloc)
            , dealloc_(dealloc)
        {
        }

        Allocator(const _Alloc & alloc, const _Dealloc & dealloc)
            : alloc_(alloc)
            , dealloc_(dealloc)
        {
        }

        explicit Allocator(const Allocator &) = default;

		static pointer allocate(std::size_t n)
		{
			_Alloc alloc;
			return static_cast<pointer>(alloc(n * sizeof (value_type)));
		}

		static void deallocate(pointer p, std::size_t n)
		{
			_Dealloc dealloc;
			dealloc(p, n * sizeof (value_type));
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

    private:
        _Alloc alloc_;
        _Dealloc dealloc_;
	};
};
