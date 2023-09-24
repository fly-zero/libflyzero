#pragma once

#include <sys/epoll.h>

#include "file_descriptor.h"

namespace flyzero {

class event_listener {
   public:
    event_listener(void) = default;

    event_listener(const event_listener &) = default;

    event_listener(event_listener &&) = default;

    virtual ~event_listener(void) = default;

    event_listener &operator=(const event_listener &) = default;

    event_listener &operator=(event_listener &&) = default;

    virtual void on_read(void) = 0;

    virtual void on_write(void) = 0;

    virtual void on_close(void) = 0;

    virtual int get_fd(void) const = 0;
};

class event_base {
   public:
    /**
     * @brief Default constructor
     */
    event_base(void) = default;

    /**
     * @brief Forbid copy
     */
    event_base(const event_base &) = delete;

    /**
     * @brief Default move constructor
     */
    event_base(event_base &&) = default;

    /**
     * @brief Forbid copy
     */
    event_base &operator=(const event_base &) = delete;

    /**
     * @brief Default move assignment
     */
    event_base &operator=(event_base &&) = default;

    /**
     * @brief Run epoll event loop
     *
     * @param max_events Max evnets buffer size
     * @param timeout Number of millionseconds that epoll_wait will block in a
     * sigle loop
     */
    void run(size_t max_events, int timeout);

    /**
     * @brief Subscribe an event
     *
     * @param listener Event listener object
     * @param events Event mask
     *
     * @return true Success
     * @return false Failed
     */
    bool subscribe(event_listener &listener, uint32_t const events) {
        epoll_event event;  // NOLINT
        event.events = events;
        event.data.ptr = &listener;

        return 0 == ::epoll_ctl(epfd_.get(), EPOLL_CTL_ADD, listener.get_fd(),
                                &event)
                   ? (++size_, true)
                   : false;
    }

    /**
     * @brief Unsubscribe an event listener
     *
     * @param listener Event listener object
     *
     * @return true Success
     * @return false Failed
     */
    bool unsubscribe(event_listener &listener) {
        return 0 == ::epoll_ctl(epfd_.get(), EPOLL_CTL_DEL, listener.get_fd(),
                                nullptr)
                   ? (--size_, true)
                   : false;
    }

    /**
     * @brief Is event_base ok
     */
    operator bool(void) const { return bool(epfd_); }

    /**
     * @brief Is event_base not ok
     */
    bool operator!(void) const { return !epfd_; }

   protected:
    /**
     * @brief Interface for dispatching the listener & evnets
     */
    virtual void on_dispacth(event_listener &listener, uint32_t events) = 0;

    virtual void on_loop(void) = 0;

    virtual void on_timeout(void) = 0;

   private:
    file_descriptor epfd_{::epoll_create1(0)};
    size_t size_{0};
};

}  // namespace flyzero