#pragma once
#ifndef OBJECT_H
#define OBJECT_H 

#include <platform.h>
#include "image.h"

#define object_initialsize (1024)
#define object_increment (128)
#define object_linitialsize (32)
#define object_lincrement (16)

enum {
	object_active = (1 << 0),
	object_hasz   = (1 << 1),
};

typedef struct {
	unsigned long  position;
	unsigned long  layer_position;
	uint_least32_t flags;
	
	double         x;
	double         y;
	uint_least8_t  z;
	double         r;
	
	image_texture* texture;
	double         offsetx;
	double         offsety;
} object;

typedef struct {
	unsigned long  number;
	unsigned long  length;
	unsigned long  size;
	unsigned long  lowest;
	object*        objects[0];
} object_layer;

extern unsigned long object_number;
extern unsigned long object_length;
extern unsigned long object_size;
extern unsigned long object_drawables;
extern unsigned long object_alayers;
extern unsigned long object_spaceused;

bool object_init();
object* object_create();
void object_makedrawable(object*, double, double, uint_least8_t, double, image_texture*, double, double);
void object_destroy(object*);

#endif
