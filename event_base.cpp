#include <cassert>
#include <cerrno>

#include "event_base.h"

namespace flyzero
{

    void event_base::run(size_t const max_events, int const timeout)
    {
        auto events = new epoll_event[max_events];

        while (true)
        {
            auto const nev = epoll_wait(epfd_.get(), events, max_events, timeout);

            if (nev == -1)
            {
                if (EINTR == errno)
                    continue;
                break;
            }

            if (0 == nev)
            {
                on_timeout();
                continue;
            }

            for (auto i = 0; i < nev; ++i)
                on_dispacth(*static_cast<event_listener *>(events[i].data.ptr), events[i].events);

            on_oop();
        }

        delete[] events;
    }

}