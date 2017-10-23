#include "Epoll.h"

namespace flyzero
{

    void Epoll::run(size_t size, int timeout, void(*onTimeout)(void *), void * arg) const
    {
        auto events = reinterpret_cast<epoll_event *>(alloc_(size * sizeof (epoll_event)));

        for (int nev; (nev = epoll_wait(epfd_.get(), events, size, timeout)) != -1; )
        {
            if (nev == 0)
            {
                if (onTimeout)
                    onTimeout(arg);
                continue;
            }

            for (int i = 0; i < nev; ++i)
            {
                IEpoll * p = static_cast<IEpoll *>(events[i].data.ptr);
                if (events[i].events & EPOLLIN)
                    p->OnRead();

                if (events[i].events & EPOLLOUT)
                    p->OnWrite();

                if (events[i].events & EPOLLRDHUP)
                    p->OnRead();
            }
        }

        dealloc_(events);
    }

}