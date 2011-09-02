#define PLATFORM_NO_INLINE
#include "platform.h"

#if defined(WIN32)

#else
#include <pthread.h>

void platform_createthread(createthread_t func, void* arg)
{
	pthread_t thread;
	pthread_create(&thread, NULL, func, arg);
}

#endif
