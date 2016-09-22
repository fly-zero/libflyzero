#pragma once

#include <array>
#include <cstring>
#include <netinet/in.h>


namespace flyzero
{
	class IPAddressV4
	{
	public:
		typedef std::array<uint8_t, 4> ByteArray4;

	private:
		union AddressStorage
		{
			static_assert(sizeof (struct in_addr) == sizeof (ByteArray4),
				"size of in_addr and ByteArray4 are different");
			struct in_addr inAddr;
			ByteArray4 bytes;
			AddressStorage(void)
			{
				memset(this, 0, sizeof (AddressStorage));
			}
		} addr;
	};
}
