#include "Thread.h"

namespace FlyZero
{
    int Thread::Start(void)
    {
        return pthread_create(&tid, nullptr, start_routine, this);
    }

    void * Thread::start_routine(void * obj)
    {
        Thread * pThread = (Thread *)(obj);
        pThread->Run();
        return nullptr;
    }
}