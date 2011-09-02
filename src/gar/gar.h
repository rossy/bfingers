#pragma once
#ifndef GAR_H
#define GAR_H

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <libchash.h>

typedef struct {
	void* gar;
	size_t length;
	struct HashTable* ht;
} gar_list;

typedef enum {
	gar_unknown,
	gar_xz_compressed,
	gar_uncompressed,
	gar_anonmap,
} gar_streamtype;

gar_streamtype gar_identify(uint8_t buffer[6]);

void* gar_map(char*, size_t*);
void gar_unmap(void*, size_t);
gar_list* gar_index(void*, size_t);
gar_list* gar_indexmap(char*);
void* gar_get(gar_list*, size_t*, const char*);

#endif
