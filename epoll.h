#pragma once

#include <sys/epoll.h>

#include <utility>
#include <cassert>
#include <functional>

#include "file_descriptor.h"

namespace flyzero
{

    class epoll_listener
    {
    public:
        virtual ~epoll_listener() = default;

        // readable event callback
        virtual void on_read() = 0;

        // writable event callback
        virtual void on_write() = 0;

        // remote close event callback, the file descriptor will be removed before callback
        virtual void on_close() = 0;

        // file descriptor getter
        virtual int get_fd() const = 0;
    };

    class epoll final
    {
    public:
        using alloc_type = std::function<void*(std::size_t)>;
        using dealloc_type = std::function<void(void *)>;

        enum event { epoll_read = EPOLLIN, epoll_write = EPOLLOUT, epoll_close = EPOLLRDHUP, epoll_edge = EPOLLET };

        epoll() = default;

        epoll(const alloc_type & alloc, const dealloc_type & dealloc)
            : epfd_(::epoll_create1(0))
            , alloc_(alloc)
            , dealloc_(dealloc)
        {
            assert(alloc);
            assert(dealloc);
        }

        epoll(const epoll & other)
            : epfd_(other.epfd_)
            , alloc_(other.alloc_)
            , dealloc_(other.dealloc_)
        {
        }

        epoll(epoll && other) noexcept
            : epfd_(std::move(other.epfd_))
            , alloc_(std::move(other.alloc_))
            , dealloc_(std::move(other.dealloc_))
        {
        }

        ~epoll() = default;

        epoll & operator=(const epoll & other)
        {
            if (this != &other)
            {
                epfd_ = other.epfd_;
                alloc_ = other.alloc_;
                dealloc_ = other.dealloc_;
            }
            return *this;
        }

        epoll & operator=(epoll && other) noexcept
        {
            if (this != &other)
            {
                epfd_ = std::move(other.epfd_);
                alloc_ = other.alloc_;
                dealloc_ = other.dealloc_;
            }
            return *this;
        }

        bool add(epoll_listener & iepoll, uint32_t const events)
        {
            epoll_event ev;
            ev.events = events;
            ev.data.ptr = &iepoll;
            return epoll_ctl(epfd_.get(), EPOLL_CTL_ADD, iepoll.get_fd(), &ev) == 0 ? (++size_, true) : false;
        }

        bool remove(const epoll_listener & iepoll)
        {
            return epoll_ctl(epfd_.get(), EPOLL_CTL_DEL, iepoll.get_fd(), nullptr) == 0 ? (--size_, true) : false;
        }

        std::size_t size(void) const
        {
            return size_;
        }

        void run(std::size_t const size, int const timeout, void (*on_timeout)(void *), void *arg) const;

    private:
        file_descriptor epfd_{ ::epoll_create1(0) };
        alloc_type alloc_{ ::malloc };
        dealloc_type dealloc_{ ::free };
        std::size_t size_{ 0 };
    };

}
