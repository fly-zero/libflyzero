#pragma once

#include <cstdint>		// c++11 is needed

namespace FlyZero
{
	class Memory
	{
	public:
		static unsigned int SearchCharacters(const void *src, unsigned int srclen, const char *str, unsigned int strlen);

		static uint64_t ReverseByteOrder(uint64_t val);
		static uint32_t ReverseByteOrder(uint32_t val);
	};
}
