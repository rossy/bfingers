#pragma once
#ifndef PLATFORM_H
#define PLATFORM_H

#include <stdlib.h>
#include <stddef.h>
#include <stdarg.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>
#include <limits.h>
#include <inttypes.h>

typedef unsigned int uint;

#if defined(__GNUC__)
	#define noreturn __attribute__((noreturn))
#else
	#define noreturn /**/
#endif

#if defined(WIN32)
	#define mem_read 0x01
	#define mem_write 0x02
	#define mem_exec 0x04
	#define mem_none 0x00
	typedef unsigned int thread_return;
#else
	#include <unistd.h>
	#include <sys/mman.h>
	#include <sys/types.h>
	#define mem_read PROT_READ
	#define mem_write PROT_WRITE
	#define mem_exec PROT_EXEC
	#define mem_none PROT_NONE
	#define stricmp strcasecmp
	typedef void* thread_return;
#endif

int asprintf(char**, const char*, ...);
int vasprintf(char **strp, const char *fmt, va_list ap);

#if defined(WIN32)
	#if defined(PLATFORM_DEFINE_TIME)
		uint64_t time_timespersec;
		uint64_t time_start;
		uint64_t time_time;
		uint64_t time_lasttime;
		uint64_t time_lastframetime;
		uint64_t time_res;
		clock_t time_cputime;
		clock_t time_lastcputime;
	#else
		extern uint64_t time_timespersec;
		extern uint64_t time_start;
		extern uint64_t time_time;
		extern uint64_t time_res;
		extern uint64_t time_lastframetime;
		extern clock_t time_lastcputime;
	#endif
	
	static inline void time_init()
	{
		LARGE_INTEGER tps;
		QueryPerformanceFrequency(&tps);
		time_timespersec = tps.QuadPart;
	}
	
#else
	#define time_timespersec (UINT64_C(1000000000))
	#define time_frametime (UINT64_C(16666667))
	#define time_cputimespersec (CLOCKS_PER_SEC)
	
	#if defined(PLATFORM_DEFINE_TIME)
		uint64_t time_start;
		uint64_t time_time;
		uint64_t time_lasttime;
		uint64_t time_lastframetime;
		uint64_t time_res;
		struct timespec time_spec;
		clock_t time_cputime;
		clock_t time_lastcputime;
	#else
		extern uint64_t time_start;
		extern uint64_t time_time;
		extern uint64_t time_res;
		extern uint64_t time_lastframetime;
		extern clock_t time_lastcputime;
	#endif
	
	static inline void time_init()
	{
		extern struct timespec time_spec;
		clock_getres(CLOCK_MONOTONIC, &time_spec);
		time_res = (uint64_t)time_spec.tv_nsec + (uint64_t)time_spec.tv_sec * time_timespersec;
		clock_gettime(CLOCK_MONOTONIC, &time_spec);
		time_start = (uint64_t)time_spec.tv_nsec + (uint64_t)time_spec.tv_sec * time_timespersec;
		time_time = time_start - time_frametime;
	}
	
	static inline void time_startframe()
	{
		extern uint64_t time_time;
		extern uint64_t time_lasttime;
		extern struct timespec time_spec;
		extern clock_t time_cputime;
		
		clock_gettime(CLOCK_MONOTONIC, &time_spec);
		time_lasttime = time_time;
		time_time = (uint64_t)time_spec.tv_nsec + (uint64_t)time_spec.tv_sec * time_timespersec;
		time_lastframetime = time_time - time_lasttime;
		if (time_lastframetime <= 0)
			time_lastframetime = 1;
		time_cputime = clock();
	}
	
	static inline void time_endframe()
	{
		extern clock_t time_cputime;
		time_lastcputime = clock() - time_cputime;
		if (time_lastcputime <= 0)
			time_lastcputime = 1;
		usleep(1);
	}
#endif

// thread.c

typedef thread_return (*createthread_t)(void*);

// platform_createthread(func, arg)
// Calls func(arg) in a new thread.
#if defined(PLATFORM_NO_INLINE)
	void platform_createthread(createthread_t, void*);
#elif defined(WIN32)
	
#else
	static inline void platform_createthread(createthread_t func, void* arg)
	{
		int pthread_create(pthread_t*, const pthread_attr_t*, createthread_t, void*);
		pthread_t thread;
		pthread_create(&thread, NULL, func, arg);
	}
#endif

// memory.c

// platform_map(filename, &length)
// Maps a local file into memory.
void* platform_map(const char*, size_t*);

// platform_map(mapping, length)
// Unmaps a file mapped by platform_map.
#if defined(PLATFORM_NO_INLINE)
	void platform_unmap(void*, size_t);
#elif defined(WIN32)
	
#else
	static inline void platform_unmap(void* mapping, size_t length)
	{
		munmap(mapping, length);
	}
#endif

// platform_virtualalloc(length, prot)
// Allocates a region of pages.
#if defined(PLATFORM_NO_INLINE)
	void* platform_virtualalloc(size_t, int);
#elif defined(WIN32)
	
#else
	static inline void* platform_virtualalloc(size_t length, int prot)
	{
		return mmap(NULL, length, prot, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
	}
#endif

// platform_virtualprotect(mem, length, prot)
// Change the permissions of a region of pages.
#if defined(PLATFORM_NO_INLINE)
	bool platform_virtualprotect(void* mem, size_t length, int prot);
#elif defined(WIN32)
	
#else
	static inline bool platform_virtualprotect(void* mem, size_t length, int prot)
	{
		return mprotect(mem, length, prot) == 0;
	}
#endif
// platform_virtualprotect(mem, length, prot)
// Frees a region of pages.
#if defined(PLATFORM_NO_INLINE)
	void platform_virtualfree(void* mem, size_t length);
#elif defined(WIN32)
	
#else
	static inline void platform_virtualfree(void* mem, size_t length)
	{
		munmap(mem, length);
	}
#endif

// file.c

extern char** config_dir;
extern char** data_dir;
extern char* config_local_dir;
extern char* data_local_dir;
extern uint config_dirs, config_maxpath, data_dirs, data_maxpath;

// platform_mkpdir(filename)
// Makes all the parent directories beneath 'filename' such that 'filename' can be sucessfully created.
bool platform_mkpdir(const char*);

// paths_init()
// Initalises config_dir, config_local_dir, data_dir and data_local_dir. Complies with XDG standards on *nix.
void paths_init();

// platform_locdup(data, basename)
// Searches for a file in the config or data directories. Returns a duplicate of the filename or NULL.
char* platform_locdup(bool, const char*);

#ifdef WIN32
#define pathsep_str "\\"
#define pathsep '\\'
#else
#define pathsep_str "/"
#define pathsep '/'
#endif

#endif
