#pragma once

#include <sys/epoll.h>

#include <cstring>

#include <algorithm>
#include <chrono>
#include <queue>
#include <system_error>
#include <vector>

#include "file_descriptor.h"

namespace flyzero {

class event_dispatch {
public:
    enum class event : int { read = EPOLLIN, write = EPOLLOUT, read_write = EPOLLIN | EPOLLOUT };

    class io_listener;

    struct loop_listener;

    struct timeout_listener;

    using time_point    = std::chrono::steady_clock::time_point;
    using time_duration = std::chrono::steady_clock::duration;

private:
    struct TimeoutListenerWrapper {
        TimeoutListenerWrapper(timeout_listener &listener, time_point now, time_duration interval);

        bool operator>(TimeoutListenerWrapper const &other) const noexcept;

        timeout_listener *listener_;
        time_point        deadline_;
        time_duration     interval_;
    };

    using timeout_listener_queue = std::priority_queue<TimeoutListenerWrapper,
                                                       std::vector<TimeoutListenerWrapper>,
                                                       std::greater<TimeoutListenerWrapper>>;

public:
    /**
     * @brief 构造函数
     */
    event_dispatch();

    /**
     * @brief 禁止拷贝
     */
    event_dispatch(const event_dispatch &) = delete;

    /**
     * @brief 禁止赋值
     */
    void operator=(const event_dispatch &) = delete;

    /**
     * @brief 移动构造函数
     */
    event_dispatch(event_dispatch &&) = default;

    /**
     * @brief 移动赋值
     */
    event_dispatch &operator=(event_dispatch &&) = default;

    /**
     * @brief 析构函数
     */
    ~event_dispatch() = default;

    /**
     * @brief 注册 IO 事件监听器
     * @param listener 监听器
     * @param event 监听的事件
     */
    void register_io_listener(io_listener &listener, event event);

    /**
     * @brief 注销 IO 事件监听器
     * @param listener 监听器
     */
    void unregister_io_listener(io_listener &listener);

    /**
     * @brief 运行事件循环
     * @param listener 监听器
     */
    void register_loop_listener(loop_listener &listener);

    /**
     * @brief 注销事件循环
     * @param listener 监听器
     */
    void unregister_loop_listener(loop_listener &listener);

    /**
     * @brief 注册超时监听器
     * @param listener 监听器
     * @param timeout 超时时间
     */
    void register_timeout_listener(timeout_listener &listener, time_duration timeout);

    /**
     * @brief 运行事件循环
     * @param timeout 超时时间
     */
    void run_loop(std::chrono::milliseconds timeout);

    /**
     * @brief 运行一次事件循环
     * @param timeout 超时时间
     */
    void run_once(std::chrono::milliseconds timeout);

protected:
    /**
     * @brief 处理循环事件
     */
    void on_loop();

    /**
     * @brief 处理超时事件
     * @param now 当前时间
     */
    void on_timeout(time_point now);

private:
    bool                         running_{false};     ///< 是否正在运行
    file_descriptor              epoll_fd_;           ///< epoll 文件描述符
    std::vector<loop_listener *> loop_listeners_;     ///< 循环监听器
    timeout_listener_queue       timeout_listeners_;  ///< 超时监听器队列
};

class event_dispatch::io_listener {
public:
    explicit io_listener(int fd);

    explicit io_listener(file_descriptor &&fd) noexcept;

    virtual ~io_listener() = default;

    int fd() const noexcept;

    virtual void on_read() = 0;

    virtual void on_write() = 0;

private:
    file_descriptor fd_;  ///< 监听的文件描述符
};

struct event_dispatch::loop_listener {
    virtual ~loop_listener() = default;

    virtual void on_loop() = 0;
};

struct event_dispatch::timeout_listener {
    virtual ~timeout_listener() = default;

    virtual bool on_timeout(time_point now) = 0;
};

inline event_dispatch::TimeoutListenerWrapper::TimeoutListenerWrapper(timeout_listener &listener,
                                                                      time_point        now,
                                                                      time_duration     interval)
    : listener_{&listener}, deadline_{now + interval}, interval_{interval} {}

inline bool event_dispatch::TimeoutListenerWrapper::operator>(
    TimeoutListenerWrapper const &other) const noexcept {
    return deadline_ > other.deadline_;
}

inline void event_dispatch::register_loop_listener(loop_listener &listener) {
    auto const it = std::find(loop_listeners_.begin(), loop_listeners_.end(), &listener);
    if (it == loop_listeners_.end()) {
        loop_listeners_.push_back(&listener);
    }
}

inline void event_dispatch::unregister_loop_listener(loop_listener &listener) {
    auto const it = std::find(loop_listeners_.begin(), loop_listeners_.end(), &listener);
    if (it != loop_listeners_.end()) {
        loop_listeners_.erase(it);
    }
}

inline void event_dispatch::register_timeout_listener(timeout_listener &listener,
                                                      time_duration     interval) {
    auto const now = std::chrono::steady_clock::now();
    timeout_listeners_.emplace(listener, now, interval);
}

inline void event_dispatch::run_loop(std::chrono::milliseconds timeout) {
    running_ = true;
    while (running_) {
        run_once(timeout);
    }
}

inline void event_dispatch::on_loop() {
    for (auto const listener : loop_listeners_) {
        listener->on_loop();
    }
}

inline event_dispatch::io_listener::io_listener(int fd) : fd_{fd} {}

inline event_dispatch::io_listener::io_listener(file_descriptor &&fd) noexcept
    : fd_{std::move(fd)} {}

inline int event_dispatch::io_listener::fd() const noexcept { return fd_.get(); }

}  // namespace flyzero