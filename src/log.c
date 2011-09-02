#include <stdio.h>
#include <platform.h>
#define LOG_NO_INLINE
#include "log.h"

void trace(const char* message)
{
	puts(message);
}

void tracef(const char* fmt, ...)
{
	char* message;
	va_list ap;
	va_start(ap, fmt);
	vasprintf(&message, fmt, ap);
	va_end(ap);
	trace(message);
	free(message);
}
