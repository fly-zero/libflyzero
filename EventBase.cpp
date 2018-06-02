#include <cassert>
#include <cerrno>

#include "EventBase.h"

namespace flyzero
{

    void EventBase::Run(size_t const max_events, int const timeout)
    {
        auto events = new epoll_event[max_events];

        while (true)
        {
            auto const nev = epoll_wait(epfd_.Get(), events, max_events, timeout);

            if (nev == -1)
            {
                if (EINTR == errno)
                    continue;
                break;
            }

            if (0 == nev)
            {
                OnTimeout();
                continue;
            }

            for (auto i = 0; i < nev; ++i)
                OnDispacth(*static_cast<EventListener *>(events[i].data.ptr), events[i].events);

            OnLoop();
        }

        delete[] events;
    }

}