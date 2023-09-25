#pragma once

#include <sys/epoll.h>

#include <chrono>
#include <cstring>
#include <queue>
#include <system_error>
#include <vector>

#include "file_descriptor.h"

namespace flyzero {

class EventDispatch {
public:
    enum class Event : int {
        read = EPOLLIN,
        write = EPOLLOUT,
        read_write = EPOLLIN | EPOLLOUT
    };

    class IoListener;

    struct LoopListener;

    struct TimeoutListener;

    using TimePoint = std::chrono::steady_clock::time_point;
    using TimeDuration = std::chrono::steady_clock::duration;

private:
    struct TimeoutListenerWrapper {
        TimeoutListenerWrapper(TimeoutListener &listener, TimePoint now,
                               TimeDuration interval);

        bool operator>(TimeoutListenerWrapper const &other) const noexcept;

        TimeoutListener *listener_;
        TimePoint deadline_;
        TimeDuration interval_;
    };

    using TimeoutListenerQueue =
        std::priority_queue<TimeoutListenerWrapper,
                            std::vector<TimeoutListenerWrapper>,
                            std::greater<TimeoutListenerWrapper>>;

public:
    /**
     * @brief 构造函数
     */
    EventDispatch();

    /**
     * @brief 禁止拷贝
     */
    EventDispatch(const EventDispatch &) = delete;

    /**
     * @brief 禁止赋值
     */
    void operator=(const EventDispatch &) = delete;

    /**
     * @brief 移动构造函数
     */
    EventDispatch(EventDispatch &&) = default;

    /**
     * @brief 移动赋值
     */
    EventDispatch &operator=(EventDispatch &&) = default;

    /**
     * @brief 析构函数
     */
    ~EventDispatch() = default;

    /**
     * @brief 注册 IO 事件监听器
     * @param listener 监听器
     * @param event 监听的事件
     */
    void register_io_listener(IoListener &listener, Event event);

    /**
     * @brief 注销 IO 事件监听器
     * @param listener 监听器
     */
    void unregister_io_listener(IoListener &listener);

    /**
     * @brief 运行事件循环
     * @param listener 监听器
     */
    void register_loop_listener(LoopListener &listener);

    /**
     * @brief 注销事件循环
     * @param listener 监听器
     */
    void unregister_loop_listener(LoopListener &listener);

    /**
     * @brief 注册超时监听器
     * @param listener 监听器
     * @param timeout 超时时间
     */
    void register_timeout_listener(TimeoutListener &listener,
                                   TimeDuration timeout);

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
    void on_timeout(TimePoint now);

private:
    bool running_{false};                         ///< 是否正在运行
    file_descriptor epoll_fd_;                    ///< epoll 文件描述符
    std::vector<LoopListener *> loop_listeners_;  ///< 循环监听器
    TimeoutListenerQueue timeout_listeners_;      ///< 超时监听器队列
};

class EventDispatch::IoListener {
public:
    explicit IoListener(int fd);

    virtual ~IoListener() = default;

    int fd() const noexcept;

    virtual void on_read() = 0;

    virtual void on_write() = 0;

private:
    file_descriptor fd_;  ///< 监听的文件描述符
};

struct EventDispatch::LoopListener {
    virtual ~LoopListener() = default;

    virtual void on_loop() = 0;
};

struct EventDispatch::TimeoutListener {
    virtual ~TimeoutListener() = default;

    virtual bool on_timeout() = 0;
};

inline EventDispatch::TimeoutListenerWrapper::TimeoutListenerWrapper(
    TimeoutListener &listener, TimePoint now, TimeDuration interval)
    : listener_{&listener}, deadline_{now + interval}, interval_{interval} {}

inline bool EventDispatch::TimeoutListenerWrapper::operator>(
    TimeoutListenerWrapper const &other) const noexcept {
    return deadline_ > other.deadline_;
}

inline void EventDispatch::register_loop_listener(LoopListener &listener) {
    auto const it =
        std::find(loop_listeners_.begin(), loop_listeners_.end(), &listener);
    if (it == loop_listeners_.end()) {
        loop_listeners_.push_back(&listener);
    }
}

inline void EventDispatch::unregister_loop_listener(LoopListener &listener) {
    auto const it =
        std::find(loop_listeners_.begin(), loop_listeners_.end(), &listener);
    if (it != loop_listeners_.end()) {
        loop_listeners_.erase(it);
    }
}

inline void EventDispatch::register_timeout_listener(TimeoutListener &listener,
                                                     TimeDuration interval) {
    auto const now = std::chrono::steady_clock::now();
    timeout_listeners_.emplace(listener, now, interval);
}

inline void EventDispatch::run_loop(std::chrono::milliseconds timeout) {
    running_ = true;
    while (running_) {
        run_once(timeout);
    }
}

inline void EventDispatch::on_loop() {
    for (auto const listener : loop_listeners_) {
        listener->on_loop();
    }
}

}  // namespace flyzero