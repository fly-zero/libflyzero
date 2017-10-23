#pragma once

#include <sys/epoll.h>

#include <utility>
#include <cstdlib>
#include <cstdio>

#include "FileDescriptor.h"

namespace flyzero
{
    class IEpoll
    {
    public:
        virtual void OnRead(void) = 0;
        virtual void OnWrite(void) = 0;
        virtual void OnClose(void) = 0;
    };

	class Epoll
	{
	public:
        typedef void * (*AllocFunc)(size_t);
        typedef void (*DeallocFunc)(void *);

        enum Event { READ = EPOLLIN, WRITE = EPOLLOUT, CLOSE = EPOLLRDHUP };

        Epoll(AllocFunc alloc = ::malloc, DeallocFunc dealloc = ::free)
            : epfd_(::epoll_create1(0))
            , alloc_(alloc)
            , dealloc_(dealloc)
        {
        }

        Epoll(const Epoll & other)
            : epfd_(other.epfd_)
            , alloc_(other.alloc_)
            , dealloc_(other.dealloc_)
        {
        }

        Epoll(Epoll && other)
            : epfd_(std::move(other.epfd_))
            , alloc_(other.alloc_)
            , dealloc_(other.dealloc_)
        {
        }

        ~Epoll(void) = default;

        Epoll & operator=(const Epoll & other)
        {
            if (this != &other)
            {
                epfd_ = other.epfd_;
                alloc_ = other.alloc_;
                dealloc_ = other.dealloc_;
            }
            return *this;
        }

        Epoll & operator=(Epoll && other)
        {
            if (this != &other)
            {
                epfd_ = std::move(other.epfd_);
                alloc_ = other.alloc_;
                dealloc_ = other.dealloc_;
            }
            return *this;
        }

        void add(IEpoll * ptr, FileDescriptor fd, uint32_t events)
        {
            epoll_event ev;
            ev.events = events;
            ev.data.ptr = ptr;
            epoll_ctl(epfd_.get(), EPOLL_CTL_ADD, fd.get(), &ev);
        }

        void run(size_t size, int timeout, void (*onTimeout)(void *), void *arg) const;

	private:
        FileDescriptor epfd_;
        AllocFunc alloc_;
        DeallocFunc dealloc_;
	};

}
