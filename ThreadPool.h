#pragma once

#include <pthread.h>
#include "Thread.h"

namespace FlyZero
{
	class ThreadPool : public Thread
	{
	public:
		ThreadPool(void);
		~ThreadPool(void);
		size_t Start(size_t nthrd);

	private:
		virtual void Run(void);

	private:
		pthread_cond_t cond;
		pthread_mutex_t mtx;
		size_t nthread{ 0 };
	};
}