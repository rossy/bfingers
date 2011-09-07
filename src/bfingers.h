#pragma once
#ifndef BFINGERS_H
#define BFINGERS_H 

#include <SDL/SDL.h>
#include <gar.h>

#define projectname "Project Butterfingers"
#define projectname_len (21)

#define projectversion "0.1"
#define projectversion_len (3)

#define shortname "bfingers"
#define shortname_len (8)

#ifndef COMPILER
#define COMPILER "unknown"
#endif

#ifndef UNAME
#define UNAME "unknown"
#endif

extern gar_list* data_root;

extern unsigned int bf_width;
extern unsigned int bf_height;

extern bool ARB_texture_non_power_of_two;
extern bool bf_drawable;

#endif
 
