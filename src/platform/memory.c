#define PLATFORM_NO_INLINE
#include "platform.h"

#if defined(WIN32)
#include <windows.h>

void* platform_map(const char* filename, size_t* length)
{
	HANDLE fd;
	HANDLE mapping;
	void* ret;
	
	if (!(fd == CreateFile(filename, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL)))
		return NULL;
	if (!(mapping == CreateFileMapping(fd, NULL, PAGE_READONLY, 0, 0, NULL)))
	{
		CloseHandle(fd);
		return NULL;
	}
	CloseHandle(fd);
	ret = MapViewOfFile(mapping, FILE_MAP_READ, 0, 0, 0);
	CloseHandle(mapping);
	return ret;
}

void platform_unmap(void* mapping, size_t length)
{
	UnmapViewOfFile(mapping);
}

void* platform_virtualalloc(size_t length, int prot)
{
	return VirtualAlloc(NULL, length, 0, (prot & mem_exec) ?
		(prot & mem_write) ? PAGE_EXECUTE_READWRITE : PAGE_EXECUTE_READ :
		(prot & mem_write) ? PAGE_READWRITE : PAGE_READONLY);
}

bool platform_virtualprotect(void* mem, size_t length, int prot)
{
	DWORD oldprot;
	return VirtualProtect(mem, length, (prot & mem_exec) ?
		(prot & mem_write) ? PAGE_EXECUTE_READWRITE : PAGE_EXECUTE_READ :
		(prot & mem_write) ? PAGE_READWRITE : PAGE_READONLY, &oldprot);
}

void platform_virtualfree(void* mem, size_t length)
{
	VirtualFree(mem, 0, MEM_RELEASE);
}

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
	{
		close(fd);
		return NULL;
	}
	
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
