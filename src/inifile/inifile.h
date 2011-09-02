#pragma once
#ifndef INIFILE_H
#define INIFILE_H

#include <stdbool.h>
#include <stdint.h>

enum {inifile_cantread = -20};

typedef bool (*inifile_callback)(char* section, bool newsection, char* name, char* value);

char* inifile_cleanstr(char* str);
bool inifile_parseint(const char*, bool, int32_t*);
bool inifile_read(const char* filename, inifile_callback callback);

#endif
