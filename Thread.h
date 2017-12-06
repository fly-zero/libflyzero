#pragma once

#include <pthread.h>

namespace FlyZero
{
    class Thread
    {
    public:
        int Start(void);

    private:
        virtual void Run(void) = 0;

    private:
        static void * start_routine(void * obj);

    private:
        pthread_t tid{ 0 };
    };
}