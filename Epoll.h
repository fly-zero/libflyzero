#pragma once

#include <sys/epoll.h>

#include <utility>
#include <cstdlib>
#include <cstdio>
#include <cassert>
#include <functional>

#include "FileDescriptor.h"

namespace flyzero
{
    class IEpoll
    {
    public:
        virtual void onRead(void) = 0;
        virtual void onWrite(void) = 0;
        virtual void onClose(void) = 0;
        virtual int getFileDescriptor(void) const = 0;
    };

	class Epoll
	{
	public:
        using alloc_type = std::function<void*(size_t)>;
        using dealloc_type = std::function<void(void *)>;

        enum Event { READ = EPOLLIN, WRITE = EPOLLOUT, CLOSE = EPOLLRDHUP, EDGE = EPOLLET };

        Epoll(void) = default;

        Epoll(alloc_type alloc, dealloc_type dealloc)
            : epfd_(::epoll_create1(0))
            , alloc_(alloc)
            , dealloc_(dealloc)
        {
            assert(alloc);
            assert(dealloc);
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

        void add(IEpoll * ptr, uint32_t events)
        {
            epoll_event ev;
            ev.events = events;
            ev.data.ptr = ptr;
            epoll_ctl(epfd_.get(), EPOLL_CTL_ADD, ptr->getFileDescriptor(), &ev);
        }

        void run(size_t size, int timeout, void (*onTimeout)(void *), void *arg) const;

	private:
        FileDescriptor epfd_{ ::epoll_create1(0) };
        alloc_type alloc_{ ::malloc };
        dealloc_type dealloc_{ ::free };
	};

}
