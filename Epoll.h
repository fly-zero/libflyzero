#pragma once

#include <sys/epoll.h>

namespace FlyZero
{
	class Epoll
	{
	public:
		int create(int flags = 0) {
			return epoll_create1(flags);
		}



	private:
		int fd{ -1 };
	};
}