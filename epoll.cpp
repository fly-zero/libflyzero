#include <iostream>

#include "epoll.h"

namespace flyzero
{

    void epoll::run(std::size_t const size, int const timeout, void(*on_timeout)(void *), void * arg) const
    {
        auto const events = reinterpret_cast<epoll_event *>(alloc_(size * sizeof (epoll_event)));

        for (int nev; (nev = epoll_wait(epfd_.get(), events, size, timeout)) != -1; )
        {
            if (nev == 0)
            {
                if (on_timeout)
                    on_timeout(arg);
                continue;
            }

            for (auto i = 0; i < nev; ++i)
            {
                auto p = static_cast<epoll_listener *>(events[i].data.ptr);

                if (events[i].events & EPOLLIN)
                    p->on_read();

                if (events[i].events & EPOLLOUT)
                    p->on_write();

                if (events[i].events & EPOLLRDHUP)
                    p->on_close();
            }
        }

        dealloc_(events);
    }

}
