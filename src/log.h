#pragma once
#ifndef LOG_H
#define LOG_H

#include <platform.h>

#if defined(LOG_NO_INLINE)
	void trace(const char* message);
#else
	static inline void trace(const char* message)
	{
		int puts(const char*);
		puts(message);
	}
#endif

#if defined(LOG_NO_INLINE)
	void tracef(const char* fmt, ...);
#else
	static inline void tracef(const char* fmt, ...)
	{
		int vasprintf(char **strp, const char *fmt, va_list ap);
		
		char* message;
		va_list ap;
		va_start(ap, fmt);
		vasprintf(&message, fmt, ap);
		va_end(ap);
		trace(message);
		free(message);
	}
#endif

#define die(reason, return)\
	{\
		trace(reason);\
		exit(return);\
	}

#endif
