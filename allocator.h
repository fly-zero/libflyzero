#pragma once

#include <new>
#include <utility>

namespace flyzero
{
	template <class _Type, class _Alloc, class _Dealloc>
	struct allocator
	{
		typedef _Type value_type;
		typedef _Type * pointer;
		typedef const _Type * const_pointer;
		typedef _Type & reference;
		typedef const _Type & const_reference;

		explicit allocator(void) {}

		explicit allocator(const allocator & other) {}

		template <class _Type1, class _Alloc1, class _Dealloc1>
		explicit allocator(const allocator<_Type1, _Alloc1, _Dealloc1> &) {}

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
	};
};
