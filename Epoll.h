#pragma once

#include <sys/epoll.h>

#include <utility>
#include <cassert>
#include <functional>

#include "FileDescriptor.h"

namespace flyzero
{

    class IEpoll
    {
    public:
        virtual ~IEpoll(void) = default;

        // readable event callback
        virtual void onRead(void) = 0;

        // writable event callback
        virtual void onWrite(void) = 0;

        // remote close event callback, the file descriptor will be removed before callback
        virtual void onClose(void) = 0;

        // file descriptor getter
        virtual int getFileDescriptor(void) const = 0;
    };

    class Epoll
    {
    public:
        using alloc_type = std::function<void*(size_t)>;
        using dealloc_type = std::function<void(void *)>;

        enum Event { READ = EPOLLIN, WRITE = EPOLLOUT, CLOSE = EPOLLRDHUP, EDGE = EPOLLET };

        Epoll(void) = default;

        Epoll(const alloc_type & alloc, const dealloc_type & dealloc)
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

        Epoll(Epoll && other) noexcept
            : epfd_(std::move(other.epfd_))
            , alloc_(std::move(other.alloc_))
            , dealloc_(std::move(other.dealloc_))
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

        Epoll & operator=(Epoll && other) noexcept
        {
            if (this != &other)
            {
                epfd_ = std::move(other.epfd_);
                alloc_ = other.alloc_;
                dealloc_ = other.dealloc_;
            }
            return *this;
        }

        void add(IEpoll & iepoll, uint32_t events)
        {
            epoll_event ev;
            ev.events = events;
            ev.data.ptr = &iepoll;
            epoll_ctl(epfd_.get(), EPOLL_CTL_ADD, iepoll.getFileDescriptor(), &ev);
            ++size_;
        }

        void remove(const IEpoll & iepoll)
        {
            epoll_ctl(epfd_.get(), EPOLL_CTL_DEL, iepoll.getFileDescriptor(), nullptr);
            --size_;
        }

        size_t size(void) const
        {
            return size_;
        }

        void run(size_t size, int timeout, void (*onTimeout)(void *), void *arg) const;

    private:
        FileDescriptor epfd_{ ::epoll_create1(0) };
        alloc_type alloc_{ ::malloc };
        dealloc_type dealloc_{ ::free };
        size_t size_{ 0 };
    };

}
