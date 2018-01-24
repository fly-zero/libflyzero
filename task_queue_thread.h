#pragma once

#include <thread>
#include <memory>
#include <cstdlib>

#include <boost/interprocess/sync/interprocess_semaphore.hpp>

#include "circular_queue.h"

namespace flyzero
{
    
    class task_queue_thread
    {
    public:
        class task_handler
        {
        public:
            task_handler(void) = default;
            virtual ~task_handler(void) = default;
            virtual void run(class task_queue_thread & thread_obj) = 0;
        };

    protected:
        using task_deleter = std::function<void(task_handler *)>;
        using task_pointer = std::unique_ptr<task_handler, task_deleter>;
        using task_queue = flyzero::circular_queue<task_pointer>;

    public:
        using alloc_type = std::function<void*(std::size_t)>;
        using dealloc_type = std::function<void(void *)>;

        explicit task_queue_thread(std::size_t queue_capacity, const alloc_type & alloc = malloc, const dealloc_type & dealloc = free)
            : queue_(queue_capacity, flyzero::allocator<task_pointer>(alloc, dealloc))
            , thread_(thread_routine, std::ref(*this))
            , processed_task_num_(0)
            , sema_(0)
        {
        }

        bool push(task_handler* th, void (*deleter)(task_handler*));

        std::size_t get_processed_task_num(void) const { return processed_task_num_; }

        std::size_t get_current_task_num(void) const { return queue_.size(); }

    protected:
        void pop(void) { queue_.pop(); }

        task_handler * front(void) { return queue_.front().get(); }

        const task_handler * front(void) const { return queue_.front().get(); }

        static void thread_routine(task_queue_thread & obj);

    private:
        task_queue queue_;
        std::thread thread_;
        std::size_t processed_task_num_;
        boost::interprocess::interprocess_semaphore sema_;
    };

}