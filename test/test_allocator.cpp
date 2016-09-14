#include "allocator.h"
#include <map>
#include <string>
#include <iostream>
#include <cstdlib>

namespace test
{

	class A
	{
	public:
		void * operator()(std::size_t n) const
		{
			auto p = malloc(n);
			if (p)
			{
				size_allocated += n;
				std::cout << "Allocate request: " << n << std::endl;
			}
			return p;
		}

		void operator()(void * p, size_t n) const
		{
			if (p)
			{
				size_allocated -= n;
				std::cout << "Deallocate request: " << n << std::endl;
			}
			free(p);
		}

		static size_t get_allocated_size(void)
		{
			return size_allocated;
		}

	private:
		static size_t size_allocated;
	};

	size_t A::size_allocated = 0;
	
	template <typename T>
	using Allocator = flyzero::allocator<T, A, A>;

	using string = std::basic_string<char, std::char_traits<char>, Allocator<char> >;

	template <typename Key, typename Type, typename Compare = std::less<Key> >
	using map = std::map<Key, Type, Compare, Allocator<std::pair<const Key, Type> > >;
};


int main(void)
{
	test::string str("hello, world!");
	str.resize(1000000);
	test::map<test::string, test::string> amap;
	amap.insert(std::make_pair(test::string("hello"), test::string("world")));
	std::cout << "Total allocated: " << test::A::get_allocated_size() << std::endl;
	return 0;
}
