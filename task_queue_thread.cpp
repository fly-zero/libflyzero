#include "task_queue_thread.h"

namespace flyzero
{

    bool task_queue_thread::push(task_handler* th, void (* deleter)(task_handler*))
    {
        if (queue_.full())
            return false;

        queue_.push(task_pointer(th, deleter));

        if (get_current_task_num() == 1)
            sema_.post();

        return true;
    }

    void task_queue_thread::thread_routine(task_queue_thread* obj)
    {
        for (; ;)
        {
            if (obj->queue_.empty())
            {
                obj->sema_.wait();
                continue;
            }

            obj->front()->run();

            obj->pop();

            ++obj->processed_task_num_;
        }
    }

}
