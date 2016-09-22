#pragma once

#include <array>
#include <cstring>
#include <netinet/in.h>

namespace flyzero
{
	class IPAddressV6
	{
	public:
		typedef std::array<uint8_t, 16> ByteArray16;

	private:
		union AddressStorage
		{
			static_assert(sizeof (struct in6_addr) == sizeof (ByteArray16),
						  "size of in6_addr and ByteArray16 are different");
			struct in6_addr inAddr;
			ByteArray16 bytes;
			AddressStorage(void)
			{
				memset(this, 0, sizeof (AddressStorage));
			}
		} addr;
	};
}
