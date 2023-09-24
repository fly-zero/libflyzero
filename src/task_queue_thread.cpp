#include "task_queue_thread.h"

namespace flyzero
{

    bool task_queue_thread::push(task * th, void (* deleter)(task*))
    {
        if (queue_.full())
            return false;

        queue_.push(task_pointer(th, deleter));

        if (get_current_task_num() == 1)
            sema_.post();

        return true;
    }

    void task_queue_thread::thread_routine(task_queue_thread & obj)
    {
        for ( ; ; )
        {
            if (obj.queue_.empty())
            {
                obj.sema_.wait();
                continue;
            }

            auto fin = obj.front()->run(obj);

            obj.pop();

            ++obj.processed_task_num_;

            if (!fin)
                break;
        }
    }

}
