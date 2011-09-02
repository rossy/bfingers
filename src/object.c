#include <platform.h>
#include "image.h"
#include "object.h"

#define array_full ULONG_MAX

object** object_list;
object_layer** object_layers;

unsigned long object_number = 0;               // number of allocated objects
unsigned long object_length = 0;               // used space in array
unsigned long object_size = 0;                 // maximum space in array
unsigned long object_drawables = 0;            // number of drawable objects
unsigned long object_alayers = 0;              // number of layers with stuff in them
unsigned long object_spaceused = 0;            // size in bytes of all allocated memory
static unsigned long lowestspace = array_full; // unallocated element with lowest index or array_full

static inline void* omalloc(size_t size)
{
	object_spaceused += size;
	return malloc(size);
}

static inline void* orealloc(void* old, size_t size, ptrdiff_t diff)
{
	object_spaceused += diff;
	return realloc(old, size);
}

static inline void ofree(void* ptr, size_t size)
{
	object_spaceused -= size;
	free(ptr);
}

bool object_init()
{
	object_list = omalloc(object_initialsize * sizeof(object*));
	object_size = object_initialsize;
	object_layers = calloc(256, sizeof(object_layer*));
	object_spaceused += 256 * sizeof(object_layer*);
	
	return true;
}

object* object_create()
{
	object* obj = omalloc(sizeof(object));
	unsigned long index;
	
	if (lowestspace == array_full)
	{
		if (++object_length > object_size)
			object_list = orealloc(object_list, (object_size += object_increment) * sizeof(object*), object_increment * sizeof(object*));
		index = object_length - 1;
	}
	else
	{
		unsigned long i;
		
		index = lowestspace;
		for (i = lowestspace + 1; i < object_length; i ++)
			if (object_list[i] == NULL)
			{
				lowestspace = i;
				goto set_object;
			}
		lowestspace = array_full;
	}
	
	set_object:
	obj->position = index;
	obj->flags    = 0;
	obj->texture  = NULL;
	
	object_list[index] = obj;
	object_number ++;
	return obj;
}

void object_makedrawable(object* obj, double x, double y, uint_least8_t z, double r, image_texture* texture, double offsetx, double offsety)
{
	obj->x = x;
	obj->y = y;
	obj->z = z;
	obj->r = r;
	
	if (!object_layers[z])
	{
		object_layers[z] = omalloc(sizeof(object_layer) + sizeof(object*) * object_linitialsize);
		object_layers[z]->size = object_linitialsize;
		object_layers[z]->number = 1;
		object_layers[z]->length = 1;
		object_layers[z]->objects[0] = obj;
		obj->layer_position = 0;
		object_alayers ++;
	}
	else
	{
		unsigned long index;
		object_layers[z]->number ++;
		
		if (object_layers[z]->lowest == array_full)
		{
			if (++object_layers[z]->length > object_layers[z]->size)
				object_layers[z] = orealloc(object_layers[z], sizeof(object_layer) + ((object_layers[z]->size += object_lincrement) * sizeof(object*)), object_lincrement * sizeof(object*));
			index = object_layers[z]->length - 1;
		}
		else
		{
			unsigned long i;
			
			index = object_layers[z]->lowest;
			for (i = index + 1; i < object_layers[z]->length; i ++)
				if (object_layers[z]->objects[i] == NULL)
				{
					object_layers[z]->lowest = i;
					goto insert_object;
				}
			object_layers[z]->lowest = array_full;
		}
		insert_object:
		object_layers[z]->objects[index] = obj;
		obj->layer_position = index;
	}
	
	object_drawables ++;
	obj->flags = obj->flags | object_hasz;
	
	obj->texture = texture;
	texture->refs ++;
	
	obj->offsetx = offsetx;
	obj->offsety = offsety;
}

void object_destroy(object* obj)
{
	object_number --;
	
	if (obj->texture)
		image_deletetexture(obj->texture);
	
	if (obj->flags & object_hasz)
	{
		object_drawables --;
		object_layer* layer = object_layers[obj->z];
		layer->number --;
		
		if (obj->layer_position == layer->length - 1)
			layer->length --;
		else
		{
			if (obj->layer_position < layer->lowest)
				layer->lowest = obj->position;
			layer->objects[obj->layer_position] = NULL;
		}
	}
	
	if (obj->position == object_length - 1)
		object_length --;
	else
	{
		if (obj->position < lowestspace)
			lowestspace = obj->position;
		object_list[obj->position] = NULL;
	}
	
	ofree(obj, sizeof(object));
	
}
