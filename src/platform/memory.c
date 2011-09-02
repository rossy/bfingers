#define PLATFORM_NO_INLINE
#include "platform.h"

#if defined(WIN32)

#else
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>

void* platform_map(const char* filename, size_t* length)
{
	int fd;
	void* ret;
	struct stat sb;
	
	if ((fd = open(filename, O_RDONLY)) == -1)
		return NULL;
	if (fstat(fd, &sb) == -1)
		return NULL;
	
	*length = sb.st_size;
	ret = mmap(NULL, sb.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
	close(fd);
	return ret;
}

void platform_unmap(void* mapping, size_t length)
{
	munmap(mapping, length);
}

void* platform_virtualalloc(size_t length, int prot)
{
	return mmap(NULL, length, prot, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
}

bool platform_virtualprotect(void* mem, size_t length, int prot)
{
	return mprotect(mem, length, prot) == 0;
}

void platform_virtualfree(void* mem, size_t length)
{
	munmap(mem, length);
}

#endif
