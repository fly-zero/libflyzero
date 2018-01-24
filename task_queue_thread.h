#pragma once

#include <thread>
#include <memory>
#include <cstdlib>

#include <boost/interprocess/sync/interprocess_semaphore.hpp>

#include "circular_queue.h"

namespace flyzero
{
    
    class task_queue_thread
        : public std::thread
    {
    public:
        class task
        {
        public:
            task(void) = default;
            virtual ~task(void) = default;
            virtual bool run(class task_queue_thread & thread_obj) = 0;
        };

        using alloc_type = std::function<void*(std::size_t)>;
        using dealloc_type = std::function<void(void *)>;

    protected:
        using task_deleter = std::function<void(task *)>;
        using task_pointer = std::unique_ptr<task, task_deleter>;
        using task_queue = flyzero::circular_queue<task_pointer>;

    public:
        explicit task_queue_thread(std::size_t queue_capacity, const alloc_type & alloc = malloc, const dealloc_type & dealloc = free)
            : std::thread(thread_routine, std::ref(*this))
            , queue_(queue_capacity, flyzero::allocator<task_pointer>(alloc, dealloc))
            , processed_task_num_(0)
            , sema_(0)
        {
        }

        bool push(task* th, void (*deleter)(task*));

        std::size_t get_processed_task_num(void) const { return processed_task_num_; }

        std::size_t get_current_task_num(void) const { return queue_.size(); }

    protected:
        void pop(void) { queue_.pop(); }

        task * front(void) { return queue_.front().get(); }

        const task * front(void) const { return queue_.front().get(); }

        static void thread_routine(task_queue_thread & obj);

    private:
        task_queue queue_;
        std::size_t processed_task_num_;
        boost::interprocess::interprocess_semaphore sema_;
    };

}