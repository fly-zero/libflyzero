#pragma once

#include <sys/epoll.h>

#include "file_descriptor.h"

namespace flyzero
{

    class EventListener
    {
    public:
        EventListener(void) = default;

        EventListener(const EventListener &) = default;

        EventListener(EventListener &&) = default;

        virtual ~EventListener(void) = default;

        EventListener & operator=(const EventListener &) = default;

        EventListener & operator=(EventListener &&) = default;

        virtual bool OnRead(void) = 0;

        virtual bool OnWrite(void) = 0;

        virtual void OnClose(void) = 0;

        virtual int GetFd(void) const = 0;
    };

    class EventBase
    {
    public:
        EventBase(void) = default;

        EventBase(const EventBase &) = delete;

        EventBase(EventBase &&) = default;

        EventBase & operator=(const EventBase &) = delete;

        EventBase & operator=(EventBase &&) = default;

        void Run(size_t max_events, int timeout);

        bool Subscribe(EventListener & listener, uint32_t events)
        {
            epoll_event event;  // NOLINT
            event.events = events;
            event.data.ptr = &listener;
            
            return 0 == ::epoll_ctl(epfd_.Get(), EPOLL_CTL_ADD, listener.GetFd(), &event) ? (++size_, true) : false;
        }

        bool Unsubscribe(EventListener & listener)
        {
            return 0 == ::epoll_ctl(epfd_.Get(), EPOLL_CTL_DEL, listener.GetFd(), nullptr) ? (--size_, true) : false;
        }

        operator bool(void) const { return bool(epfd_); }

        bool operator!(void) const { return !epfd_; }

    protected:
        virtual void OnDispacth(EventListener & listener, uint32_t events) = 0;

        virtual void OnLoop(void) = 0;

        virtual void OnTimeout(void) = 0;

   private:
        file_descriptor epfd_ { ::epoll_create1(0) };
        size_t size_{ 0 };
    };

}