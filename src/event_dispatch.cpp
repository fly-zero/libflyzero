#include "event_dispatch.h"

#include "utility.h"

namespace flyzero {

event_dispatch::event_dispatch() : epoll_fd_{::epoll_create1(EPOLL_CLOEXEC)} {
    if (!epoll_fd_) {
        throw utility::system_error(
            errno, "epoll_create1(EPOLL_CLOEXEC) failed: %s", std::strerror(errno));
    }
}

void event_dispatch::register_io_listener(io_listener &listener, event event) {
    epoll_event ev;
    ev.events      = static_cast<int>(event) | EPOLLET;
    ev.data.ptr    = &listener;
    auto const err = ::epoll_ctl(epoll_fd_.get(), EPOLL_CTL_ADD, listener.fd(), &ev);
    if (err != 0) {
        throw utility::system_error(errno,
                                    "epoll_ctl(%d, EPOLL_CTL_ADD, %d, %p) failed: %s",
                                    epoll_fd_.get(),
                                    listener.fd(),
                                    &listener,
                                    std::strerror(errno));
    }
}

void event_dispatch::unregister_io_listener(io_listener &listener) {
    auto const err = ::epoll_ctl(epoll_fd_.get(), EPOLL_CTL_DEL, listener.fd(), nullptr);
    if (err != 0) {
        throw utility::system_error(errno,
                                    "epoll_ctl(%d, EPOLL_CTL_DEL, %d, nullptr) failed: %s",
                                    epoll_fd_.get(),
                                    listener.fd(),
                                    std::strerror(errno));
    }
}

void event_dispatch::run_once(std::chrono::milliseconds timeout) {
    // 处理循环事件
    on_loop();

    // 等待 IO 事件
    constexpr int const max_events = 64;
    epoll_event         events[max_events];
    auto const          n = ::epoll_wait(epoll_fd_.get(), events, max_events, timeout.count());
    if (n < 0) {
        if (errno == EINTR) {
            return;  // 被信号中断，继续等待
        }

        throw utility::system_error(errno,
                                    "epoll_wait(%d, %p, %d, -1) failed: %s",
                                    epoll_fd_.get(),
                                    events,
                                    max_events,
                                    std::strerror(errno));
    }

    // 处理超时事件
    if (n == 0) {
        on_timeout(std::chrono::steady_clock::now());
        return;
    }

    // 处理 IO 事件
    for (int i = 0; i < n; ++i) {
        auto const listener = static_cast<io_listener *>(events[i].data.ptr);
        if (events[i].events & EPOLLIN) {
            listener->on_read();
        }

        if (events[i].events & EPOLLOUT) {
            listener->on_write();
        }
    }
}

void event_dispatch::on_timeout(time_point now) {
    while (!timeout_listeners_.empty()) {
        auto const &wrapper = timeout_listeners_.top();
        if (wrapper.deadline_ > now) break;

        auto const result = wrapper.listener_->on_timeout(now);
        if (result) {
            timeout_listeners_.emplace(*wrapper.listener_, now, wrapper.interval_);
        }

        timeout_listeners_.pop();
    }
}

}  // namespace flyzero