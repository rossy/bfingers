#pragma once
#ifndef SETTINGS_H
#define SETTINGS_H 

#include <stdbool.h>

#define configfile "config"
#define configfile_len 6

bool settings_init(int, char**);

// Settings

extern bool    options_version       ;

extern int32_t display_width         ;
extern int32_t display_height        ;
extern int32_t display_depth         ;
extern bool    display_fullscreen    ;
extern bool    display_border        ;
extern bool    display_doublebuffer  ;
extern bool    display_waitforvblank ;
extern bool    display_printfps      ;

extern char*   data_location         ;

// End of settings

#endif
