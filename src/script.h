#pragma once
#ifndef SCRIPT_H
#define SCRIPT_H

#include <stdbool.h>
#include <lua.h>
#include "script.h" 

extern lua_State *script_state;

bool script_init();

bool script_runbuf(void*, size_t);

#endif
