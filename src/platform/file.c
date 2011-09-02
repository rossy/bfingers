#define _GNU_SOURCE
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include "platform.h"
#include "../bfingers.h"

char** config_dir = NULL;
char** data_dir = NULL;
char* config_local_dir = NULL;
char* data_local_dir = NULL;
uint config_dirs, config_maxpath = 0, data_dirs, data_maxpath = 0;

#if defined(__unix)
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

static void paths_uninit()
{
	if (config_dir)
	{
		uint i;
		for (i = 0; i < config_dirs; i ++)
			free(config_dir[i]);
		free(config_dir);
	}
	
	if (data_dir)
	{
		uint i;
		for (i = 0; i < data_dirs; i ++)
			free(data_dir[i]);
		free(data_dir);
	}
}

void paths_init()
{
	char* xdgdir;
	uint i;
	config_dirs = 2;
	data_dirs = 2;
	
	atexit(paths_uninit);
	
	if ((xdgdir = getenv("XDG_CONFIG_HOME")) && *xdgdir)
		asprintf(&config_local_dir, "%s/" shortname, xdgdir);
	else if ((xdgdir = getenv("HOME")))
		asprintf(&config_local_dir, "%s/.config/" shortname, xdgdir);
	else
		config_local_dir = strdup(".");
	
	config_maxpath = strlen(config_local_dir);
	
	if ((xdgdir = getenv("XDG_DATA_HOME")) && *xdgdir)
		asprintf(&data_local_dir, "%s/" shortname, xdgdir);
	else if ((xdgdir = getenv("HOME")))
		asprintf(&data_local_dir, "%s/.local/share/" shortname, xdgdir);
	else
		data_local_dir = strdup(".");
	
	data_maxpath = strlen(data_local_dir);
	
	if ((xdgdir = getenv("XDG_CONFIG_DIRS")) && *xdgdir)
	{
		uint curlen = 0;
		
		for (i = 0;; curlen ++, i ++)
			if (xdgdir[i] == ':')
			{
				xdgdir[i] = '\0';
				if (curlen > config_maxpath)
					config_maxpath = curlen + shortname_len + 1;
				curlen = 0;
				config_dirs ++;
			}
			else if (xdgdir[i] == '\0')
				break;
		
		if (curlen > config_maxpath)
			config_maxpath = curlen + shortname_len;
		
		config_dir = calloc(config_dirs, sizeof(char*));
		config_dir[0] = config_local_dir;
		for (i = 1; i < config_dirs; i ++)
		{
			asprintf(&config_dir[i], "%s/" shortname, xdgdir);
			while (*xdgdir++ != '\0')
				;
		}
	}
	else
	{
		config_dir = calloc(2, sizeof(char*));
		config_dir[0] = config_local_dir;
		config_dir[1] = strdup("/etc/xdg/" shortname);
		if ((shortname_len + 9) > config_maxpath)
			config_maxpath = shortname_len + 9;
	}
	
	if ((xdgdir = getenv("XDG_DATA_DIRS")) && *xdgdir)
	{
		uint curlen = 0;
		
		for (i = 0;; curlen ++, i ++)
			if (xdgdir[i] == ':')
			{
				xdgdir[i] = '\0';
				if (curlen > data_maxpath)
					data_maxpath = curlen + shortname_len + 1;
				curlen = 0;
				data_dirs ++;
			}
			else if (xdgdir[i] == '\0')
				break;
		
		if (curlen > data_maxpath)
			data_maxpath = curlen + shortname_len;
		
		data_dir = calloc(data_dirs, sizeof(char*));
		data_dir[0] = data_local_dir;
		for (i = 1; i < data_dirs; i ++)
		{
			asprintf(&data_dir[i], "%s/" shortname, xdgdir);
			while (*xdgdir++ != '\0')
				;
		}
	}
	else
	{
		data_dirs = 3;
		data_dir = calloc(3, sizeof(char*));
		data_dir[0] = data_local_dir;
		data_dir[1] = strdup("/usr/local/share" shortname);
		data_dir[2] = strdup("/usr/share" shortname);
		if ((shortname_len + 16) > data_maxpath)
			data_maxpath = shortname_len + 16;
	}
}

bool platform_mkpdir(const char* filename)
{
	char* buf = strdup(filename);
	uint i = 0;
	struct stat buffer;
	
	if (*buf == pathsep)
		i = 1;
	for (; buf[i]; i ++)
		if (buf[i] == pathsep)
		{
			char tmp = buf[i];
			buf[i] = '\0';
			if (lstat(buf, &buffer) || !(buffer.st_mode & S_IFDIR))
				if (!mkdir(buf, 0777))
				{
					free(buf);
					return false;
				}
			buf[i] = tmp;
		}
	
	free(buf);
	return true;
}

bool platform_fileexists(const char* filename)
{
	struct stat buffer;
	return !lstat(filename, &buffer) && !(buffer.st_mode & S_IFDIR);
}

char* platform_locdup(bool data, const char* basename)
{
	char* buf;
	uint i, len, bn_len = strlen(basename);
	if (data)
	{
		buf = malloc(data_maxpath + bn_len + 2);
		for (i = 0; i < data_dirs; i ++)
		{
			len = strlen(data_dir[i]);
			memcpy(buf, data_dir[i], len);
			buf[len] = pathsep;
			memcpy(buf + len + 1, basename, bn_len + 1);
			if (platform_fileexists(buf))
				return buf;
		}
	}
	else
	{
		buf = malloc(config_maxpath + bn_len + 2);
		for (i = 0; i < config_dirs; i ++)
		{
			len = strlen(config_dir[i]);
			memcpy(buf, config_dir[i], len);
			buf[len] = pathsep;
			memcpy(buf + len + 1, basename, bn_len + 1);
			if (platform_fileexists(buf))
				return buf;
		}
	}
	free(buf);
	return NULL;
}

#endif
