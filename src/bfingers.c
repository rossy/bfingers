#include <png.h>
#define PLATFORM_DEFINE_TIME
#include <platform.h>
#include <stdio.h>
#include <time.h>
#include <SDL/SDL.h>
#include <GL/gl.h>
#include <gar.h>
#include "bfingers.h"
#include "log.h"
#include "image.h"
#include "script.h"
#include "settings.h"
#include "object.h"

gar_list* data_root;

unsigned int bf_width = 0, bf_height = 0;
bool bf_drawable = false;

bool ARB_texture_non_power_of_two = false;

double time_lastfps = 60.0, time_avgfps = 60.0, time_avgcputime = 0.0, time_maxcputime = 0.0;
unsigned int time_ftilwipemax = 60, time_ftilprint = 10;
unsigned long time_frames = 0;

static SDL_Surface* screen = NULL;

static void bfexit()
{
	SDL_Quit();
	tracef("%3.2f frames/sec                                                  ", (double)time_frames / ((double)(time_time - time_start) / (double)time_timespersec));
	trace("Exited");
}

static void resize()
{
	glViewport(0, 0, bf_width, bf_height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	
	glOrtho(0, bf_width, bf_height, 0, -1, 1);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	
	lua_getfield(script_state, LUA_REGISTRYINDEX, "Display");
	lua_pushinteger(script_state, bf_width);
	lua_setfield(script_state, -2, "width");
	lua_pushinteger(script_state, bf_height);
	lua_setfield(script_state, -2, "height");
	lua_pop(script_state, 1);
	
	lua_getglobal(script_state, "onResize");
	lua_call(script_state, 0, 0);
	lua_pop(script_state, lua_gettop(script_state));
}

void bf_resize(unsigned int width, unsigned int height)
{
	screen = SDL_SetVideoMode(bf_width = width, bf_height = height, display_depth, SDL_OPENGL | SDL_RESIZABLE | (display_fullscreen ? SDL_FULLSCREEN : 0));
	resize();
}

static thread_return screenshot_writepng(void* buf)
{
	static unsigned long scrnum = 0;
	unsigned long pitch = bf_width * 3, seconds = time(NULL);
	long i;
	char* filename = NULL;
	png_structp png_ptr;
	png_infop info_ptr;
	FILE *fp;
	
	asprintf(&filename, shortname "_screenshot_%010lu_%03lu.png", seconds, ++scrnum);
	tracef("Saving screenshot to %s", filename);
	
	if (!(fp = fopen(filename, "wb")))
	{
		free(buf);
		free(filename);
		return 0;
	}
	free(filename);
	
	if (!(png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL)))
	{
		free(buf);
		fclose(fp);
		return 0;
	}
	
	if (!(info_ptr = png_create_info_struct(png_ptr)))
	{
		free(buf);
		png_destroy_write_struct(&png_ptr, NULL);
		fclose(fp);
		return 0;
	}
	
	if (setjmp(png_jmpbuf(png_ptr)))
	{
		free(buf);
		png_destroy_write_struct(&png_ptr, &info_ptr);
		fclose(fp);
		return 0;
	}
	
	png_init_io(png_ptr, fp);
	png_set_compression_level(png_ptr, Z_NO_COMPRESSION);
	png_set_IHDR(png_ptr, info_ptr, bf_width, bf_height, 8, PNG_COLOR_TYPE_RGB, PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
	png_write_info(png_ptr, info_ptr);
	
	for (i = bf_height * pitch - pitch; i >= 0; i -= pitch)
		png_write_row(png_ptr, buf + i);
	
	free(buf);
	
	png_write_end(png_ptr, info_ptr);
	png_destroy_write_struct(&png_ptr, &info_ptr);
	fclose(fp);
	
	return 0;
}

static inline void save_screenshot()
{
	void* buf = malloc(bf_height * bf_width * 3);
	glReadPixels(0, 0, bf_width, bf_height, GL_RGB, GL_UNSIGNED_BYTE, buf);
	
	platform_createthread(screenshot_writepng, buf);
}

static inline void process_gl_extensions(const char* extensions)
{
	char* ext_dup = strdup(extensions), * pos = ext_dup, * ext;
	while ((ext = strsep(&pos, " ")))
		if (!strcmp(ext, "GL_ARB_texture_non_power_of_two"))
			ARB_texture_non_power_of_two = true;
	free(ext_dup);
}

int main(int argc, char** argv)
{
	SDL_Event event;
	SDL_version sdl_compiled;
	SDL_version const* sdl_linked = SDL_Linked_Version();
	char* dataname, * windowtitle;
	void* initscript;
	double time_dcputime;
	size_t size;
	
	trace(" --- " projectname " ---\n          version: " projectversion "\n         compiler: " COMPILER "\n      compiled on: " UNAME);
	
	paths_init();
	if (!settings_init(argc, argv))
		return 2;
	if (options_version)
		return 0;
	object_init();
	script_init();
	
	if (!(dataname = data_location ? data_location : platform_locdup(true, "data.gar")))
		die("Could not find data.", 2);
	
	if (dataname && !(data_root = gar_indexmap(dataname)))
		die("Failed to load data.", 2);
	
	tracef("        data root: %s", dataname);
	
	SDL_VERSION(&sdl_compiled);
	tracef(" --- SDL ---\n compiled version: %d.%d.%d\n   linked version: %d.%d.%d", sdl_compiled.major, sdl_compiled.minor, sdl_compiled.patch, sdl_linked->major, sdl_linked->minor, sdl_linked->patch);
	
	if (SDL_Init(SDL_INIT_VIDEO) < 0)
		die("SDL_Init failed :(", 1);
	SDL_putenv("SDL_VIDEO_CENTERED=center");
	
	if ((windowtitle = gar_get(data_root, NULL, "WindowTitle")))
		SDL_WM_SetCaption(windowtitle, NULL);
	else
		SDL_WM_SetCaption(projectname, NULL);
	
	SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, display_doublebuffer ? 1 : 0);
	SDL_GL_SetAttribute(SDL_GL_SWAP_CONTROL, display_waitforvblank ? 1 : 0);
	
	if (!(screen = SDL_SetVideoMode(bf_width = display_width, bf_height = display_height, display_depth, SDL_OPENGL | SDL_RESIZABLE | (display_fullscreen ? SDL_FULLSCREEN : 0))))
		die("Could not get display surface :(", 1);
	atexit(bfexit);
	
	tracef(" --- OpenGL ---\n         renderer: %s\n          version: %s\n           vendor: %s", glGetString(GL_RENDERER), glGetString(GL_VERSION), glGetString(GL_VENDOR));
	process_gl_extensions((const char*)glGetString(GL_EXTENSIONS));
	if (!ARB_texture_non_power_of_two)
		die("Error: Requires GL_ARB_texture_non_power_of_two.", 3);
	
	resize();
	
	glEnable(GL_BLEND);
	glEnable(GL_TEXTURE_2D);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glClearColor(0, 0, 0, 0);
	
	if ((initscript = gar_get(data_root, &size, "Scripts.Init")))
		script_runbuf(initscript, size);
	
	time_init();
	for (;;)
	{
		time_startframe();
		time_frames ++;
		if (display_printfps)
		{
			time_lastfps = (double)time_timespersec / (double)time_lastframetime;
			time_avgfps = (time_lastfps + time_avgfps * 9.0) / 10.0;
			time_dcputime = (double)time_lastcputime / (double)time_cputimespersec * 1000.0;
			time_avgcputime = (time_dcputime + time_avgcputime) * 9.0 / 10.0;
			if (time_dcputime > time_maxcputime)
				time_maxcputime = time_dcputime;
			if (!--time_ftilprint)
			{
				time_ftilprint = 10;
				printf("%3.2ffps, %01.3fms %01.3fmax %luo %lus %lua %lul %lud %lubytes          \r", time_avgfps, time_avgcputime, time_maxcputime, object_number, object_length, object_size, object_alayers, object_drawables, object_spaceused);
			}
			fflush(stdout);
			if (!--time_ftilwipemax)
			{
				time_ftilwipemax = 60;
				time_maxcputime = 0.0;
			}
		}
		
		while (SDL_PollEvent(&event))
			switch (event.type)
			{
				case SDL_VIDEORESIZE:
					bf_resize(event.resize.w, event.resize.h);
					break;
				case SDL_QUIT:
					return 0;
				case SDL_KEYDOWN:
					switch (event.key.keysym.sym)
					{
						case SDLK_PRINT:
						case SDLK_F5:
							save_screenshot();
							break;
						case SDLK_ESCAPE:
							return 0;
						default:
							break;
					}
				default:
					break;
			}
		
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		
		bf_drawable = true;
		lua_getglobal(script_state, "onDraw");
		lua_call(script_state, 0, 0);
		lua_pop(script_state, lua_gettop(script_state));
		bf_drawable = false;
		
		time_endframe();
		SDL_GL_SwapBuffers();
	}
}
