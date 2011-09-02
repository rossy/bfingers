#pragma once
#ifndef IMAGE_H
#define IMAGE_H 

#include <stdint.h>
#include <SDL/SDL.h>
#include <GL/gl.h>

#if SDL_BYTEORDER == SDL_BIG_ENDIAN
	//todo
#else
	#define ltoh64(i) (i)
	#define ltoh32(i) (i)
	#define ltoh16(i) (i)
	#define ltoh8(i)  (i)
	#define htol64(i) (i)
	#define htol32(i) (i)
	#define htol16(i) (i)
	#define htol8(i)  (i)
#endif

typedef struct {
	uint32_t filesz;
	uint16_t creator1;
	uint16_t creator2;
	uint32_t bmp_offset;
} bmpfile_header;

typedef struct {
	uint32_t header_sz;
	int32_t  width;
	int32_t  height;
	uint16_t nplanes;
	uint16_t bitspp;
	uint32_t compress_type;
	uint32_t bmp_bytesz;
	int32_t  hres;
	int32_t  vres;
	uint32_t ncolors;
	uint32_t nimpcolors;
} bmpfile_infoheader;

typedef struct {
	unsigned long refs;
	unsigned long width;
	unsigned long height;
	unsigned long texwidth;
	unsigned long texheight;
	double ratx;
	double raty;
	GLuint texture;
} image_texture;

image_texture* image_totexture(uint8_t*);
void image_drawtexture(image_texture*, double, double);
void image_drawscale(image_texture*, double, double, double, double);
void image_drawrotate(image_texture*, double, double, double, double, double, double, double);
void image_deletetexture(image_texture*);

#endif
