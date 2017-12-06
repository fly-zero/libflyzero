#include "ThreadPool.h"

namespace FlyZero
{
    ThreadPool::ThreadPool(void)
    {
        pthread_cond_init(&cond, nullptr);
        pthread_mutex_init(&mtx, nullptr);
    }

    ThreadPool::~ThreadPool(void)
    {
        pthread_mutex_destroy(&mtx);
        pthread_cond_destroy(&cond);
    }

    size_t ThreadPool::Start(size_t nthrd)
    {
        for (unsigned int i = 0; i < nthrd; ++i)
            if (0 == Thread::Start())
                ++nthread;
        return nthread;
    }

    void ThreadPool::Run(void)
    {
        pthread_mutex_lock(&mtx);

        do {
            pthread_cond_wait(&cond, &mtx);
        } while (false);

        pthread_mutex_unlock(&mtx);
    }
}