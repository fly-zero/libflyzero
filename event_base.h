#pragma once

#include <sys/epoll.h>

#include "file_descriptor.h"

namespace flyzero
{

    class event_listener
    {
    public:
        event_listener(void) = default;

        event_listener(const event_listener &) = default;

        event_listener(event_listener &&) = default;

        virtual ~event_listener(void) = default;

        event_listener & operator=(const event_listener &) = default;

        event_listener & operator=(event_listener &&) = default;

        virtual bool on_read(void) = 0;

        virtual bool on_write(void) = 0;

        virtual bool on_close(void) = 0;

        virtual int get_fd(void) const = 0;
    };

    class event_base
    {
    public:
        event_base(void) = default;

        event_base(const event_base &) = delete;

        event_base(event_base &&) = default;

        event_base & operator=(const event_base &) = delete;

        event_base & operator=(event_base &&) = default;

        void run(size_t max_events, int timeout);

        bool subscribe(event_listener & listener, uint32_t events)
        {
            epoll_event event;  // NOLINT
            event.events = events;
            event.data.ptr = &listener;
            
            return 0 == epoll_ctl(epfd_.get(), EPOLL_CTL_ADD, listener.get_fd(), &event) ? (++size_, true) : false;
        }

        bool unsubscribe(event_listener & listener)
        {
            return 0 == epoll_ctl(epfd_.get(), EPOLL_CTL_DEL, listener.get_fd(), nullptr) ? (--size_, true) : false;
        }

        operator bool(void) const { return bool(epfd_); }

        bool operator!(void) const { return !epfd_; }

    protected:
        virtual void on_dispacth(event_listener & listener, uint32_t events) = 0;

        virtual void on_loop(void) = 0;

        virtual void on_timeout(void) = 0;

    private:
        file_descriptor epfd_ { epoll_create1(0) };
        size_t size_{ 0 };
    };

}