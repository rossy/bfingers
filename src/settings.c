#define _GNU_SOURCE
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <platform.h>
#include "bfingers.h"
#include "settings.h"
#include "log.h"
#include "inifile/inifile.h"

enum {s_unknown, s_options, s_display, s_data} ini_section;

// Settings

bool    options_version       = false ;

int32_t display_width         = 1024  ;
int32_t display_height        = 576   ;
int32_t display_depth         = 0     ;
bool    display_fullscreen    = false ;
bool    display_doublebuffer  = true  ;
bool    display_waitforvblank = true  ;
bool    display_printfps      = false ;

char*   data_location         = NULL  ;

// End of settings

bool settings_callback(char* section, bool newsection, char* name, char* value)
{
	if (newsection)
	{
		if (!stricmp(section, "options"))
			ini_section = s_options;
		else if (!stricmp(section, "display"))
			ini_section = s_display;
		else if (!stricmp(section, "data"))
			ini_section = s_data;
		else
		{
			ini_section = s_unknown;
			tracef("Unrecognised config section \"%s\"", section);
		}
	}
	switch (ini_section)
	{
		case s_options:
			if (!stricmp(name, "w"))
			{
				if (!inifile_parseint(value, false, &display_width))
					return false;
			}
			else if (!stricmp(name, "h"))
			{
				if (!inifile_parseint(value, false, &display_height))
					return false;
			}
			else if (!stricmp(name, "d"))
			{
				if (!inifile_parseint(value, false, &display_depth))
					return false;
			}
			else if (!stricmp(name, "fs"))
				display_fullscreen = !stricmp(inifile_cleanstr(value), "true");
			else if (!stricmp(name, "i"))
				data_location = strdup(value);
			else if (!stricmp(name, "fps"))
				display_printfps = !stricmp(inifile_cleanstr(value), "true");
			else if (!stricmp(name, "version"))
				options_version = !stricmp(inifile_cleanstr(value), "true");
			else
				tracef("Unrecognised option \"%s\"", name);
			break;
		case s_display:
			if (!stricmp(name, "width"))
			{
				if (!inifile_parseint(value, false, &display_width))
					return false;
			}
			else if (!stricmp(name, "height"))
			{
				if (!inifile_parseint(value, false, &display_height))
					return false;
			}
			else if (!stricmp(name, "depth"))
			{
				if (!inifile_parseint(value, false, &display_depth))
					return false;
			}
			else if (!stricmp(name, "fullscreen"))
				display_fullscreen = !stricmp(inifile_cleanstr(value), "true");
			else if (!stricmp(name, "doublebuffer"))
				display_doublebuffer = !stricmp(inifile_cleanstr(value), "true");
			else if (!stricmp(name, "waitforvblank"))
				display_waitforvblank = !stricmp(inifile_cleanstr(value), "true");
			else if (!stricmp(name, "printfps"))
				display_printfps = !stricmp(inifile_cleanstr(value), "true");
			else
				tracef("Unrecognised option \"%s\"", name);
			break;
		case s_data:
			if (!stricmp(name, "location"))
				data_location = strdup(value);
			else
				tracef("Unrecognised option \"%s\"", name);
			break;
		default:
			break;
	}
	
	return true;
}

static void settings_uninit()
{
	if (data_location)
		free(data_location);
}

bool settings_init(int argc, char** argv)
{
	char* ininame;
	uint len;
	
	atexit(settings_uninit);
	
	if (!(ininame = platform_locdup(false, configfile)))
	{
		FILE* fp;
		
		len = strlen(config_local_dir);
		ininame = malloc(len + configfile_len + 2);
		memcpy(ininame, config_local_dir, len);
		memcpy(ininame + len, pathsep_str configfile, 2 + configfile_len);
		if (!platform_mkpdir(ininame))
		{
			trace("Error: Couldn't create config directory.");
			free(ininame);
			return false;
		}
		trace("Creating new blank config file.");
		if (!(fp = fopen(ininame, "w")))
		{
			trace("Error: Couldn't create config file.");
			free(ininame);
			return false;
		}
		fputs("[Data]\n", fp);
		fclose(fp);
	}
	
	if (!inifile_read(ininame, settings_callback))
		trace("Error reading config file, check your syntax :)");
	{
		int i;
		char* section;
		char* option;
		bool falseoption;
		for (i = 1; i < argc; i ++)
		{
			if (argv[i][0] == '-')
			{
				section = argv[i] + 1;
				if ((option = strchr(argv[i], '.')))
				{
					*option = '\0';
					falseoption = (option[1] == 'n' || option[1] == 'N') && (option[2] = 'o' || option[2] == 'O');
					if (i < (argc - 1) && argv[i + 1][0] != '-')
						settings_callback(section, true, option + 1, argv[++i]);
					else
						settings_callback(section, true, option + (falseoption ? 3 : 1), (falseoption ? "false" : "true"));
					*option = '.';
				}
				else
				{
					falseoption = (section[0] == 'n' || section[0] == 'N') && (section[1] = 'o' || section[1] == 'O');
					ini_section = s_options;
					if (i < (argc - 1) && argv[i + 1][0] != '-')
						settings_callback(NULL, false, section, argv[++i]);
					else
						settings_callback(NULL, false, section + (falseoption ? 2 : 0), (falseoption ? "false" : "true"));
				}
			}
			else
			{
				tracef("Parse error \"%s\"", argv[i]);
				break;
			}
		}
	}
	free(ininame);
	return true;
}
