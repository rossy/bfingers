#include <stdlib.h>
#include <string.h>
#include <lzma.h>
#include <platform.h>
#include "gar.h"

gar_streamtype gar_identify(uint8_t buffer[6])
{
	if (buffer[0] == 0xfd && buffer[1] == '7' && buffer[2] == 'z' && buffer[3] == 'X' && buffer[4] == 'Z' && buffer[5] == 0)
		return gar_xz_compressed;
	if (buffer[0] == 'd' && buffer[1] == 'a' && buffer[2] == 't' && buffer[3] == 'a' && buffer[4] == '/' && buffer[5] == 0)
		return gar_uncompressed;
	if (buffer[0] == 'a' && buffer[1] == 'n' && buffer[2] == 'o' && buffer[3] == 'n' && buffer[4] == '/' && buffer[5] == 0)
		return gar_anonmap;
	return gar_unknown;
}

void* gar_map(char* filename, size_t* out_length)
{
	uint8_t* in_data;
	size_t in_length;
	
	if (!(in_data = platform_map(filename, &in_length)))
		return NULL;
	
	switch (gar_identify(in_data))
	{
		case gar_uncompressed:
			*out_length = in_length;
			return in_data;
		case gar_xz_compressed:
		{
			uint8_t* out_data;
			size_t in_pos = 0;
			size_t out_pos = 0;
			uint8_t* ptr;
			uint64_t memlimit = 134217728;
			lzma_stream_flags flags;
			lzma_index *index;
			
			if ((ptr = in_data + in_length - 12) < in_data)
				goto error;
			
			if (lzma_stream_footer_decode(&flags, ptr) != LZMA_OK)
				goto error;
			
			if ((ptr -= flags.backward_size) < in_data)
				goto error;
			
			if (lzma_index_buffer_decode(&index, &memlimit, NULL, ptr, &in_pos, in_length - (ptr - in_data)) != LZMA_OK)
				goto error;
			
			memlimit = 134217728;
			*out_length = lzma_index_uncompressed_size(index);
			
			if (!(out_data = platform_virtualalloc(*out_length, mem_read | mem_write)))
			{
				lzma_index_end(index, NULL);
				goto error;
			}
			
			in_pos = 0;
			
			if (lzma_stream_buffer_decode(&memlimit, 0, NULL, in_data, &in_pos, in_length, out_data, &out_pos, *out_length) == LZMA_OK)
			{
				lzma_index_end(index, NULL);
				platform_unmap(in_data, in_length);
				
				if (gar_identify(out_data) != gar_uncompressed)
				{
					platform_virtualfree(out_data, *out_length);
					return NULL;
				}
				
				out_data[0] = 'a';
				out_data[1] = 'n';
				out_data[2] = 'o';
				out_data[3] = 'n';
				platform_virtualprotect(out_data, *out_length, mem_read);
				return out_data;
			}
			platform_virtualfree(out_data, *out_length);
			
			error:
			platform_unmap(in_data, in_length);
			return NULL;
		}
		default:
			platform_unmap(in_data, in_length);
			return NULL;
	}
}

gar_list* gar_index(void* gar, size_t length)
{
	gar_list* list = malloc(sizeof(gar_list));
	size_t size;
	char name[100];
	unsigned long i;
	name[99] = '\0';
	list->gar = gar;
	list->length = length;
	list->ht = AllocateHashTable(0, 1);
	for (; length >= 512; gar += 512, length -= 512)
		if (!*(char*)gar)
			return list;
		else
		{
			size = strtol(gar + 124, NULL, 8);
			if (((char*)gar)[156] == '0' || ((char*)gar)[156] == '\0')
			{
				for (i = 5; i < 99; i ++)
					switch ((name[i - 5] = ((char*)gar)[i]))
					{
						case '/':
							name[i - 5] = '.';
							break;
						case ' ':
							name[i - 5] = '\0';
						case '\0':
							goto copied;
					}
				copied:
				HashInsert(list->ht, PTR_KEY(list->ht, name), (ulong)gar + 512);
			}
			size = size ? ((size - 1) / 512 + 1) * 512 : 0;
			if (length < size)
				break;
			gar += size;
			length -= size;
		}
	FreeHashTable(list->ht);
	free(list);
	return NULL;
}

void* gar_get(gar_list* list, size_t* size, const char* name)
{
	HTItem* item = HashFind(list->ht, PTR_KEY(list->ht, name));
	if (item)
	{
		if (size)
			*size = strtol(((void*)item->data) - 388, NULL, 8);
		return (void*)item->data;
	}
	return NULL;
}

gar_list* gar_indexmap(char* filename)
{
	size_t length;
	gar_list* ret;
	void* gar = gar_map(filename, &length);
	if (!gar)
		return NULL;
	if (!(ret = gar_index(gar, length)))
		gar_unmap(gar, length);
	return ret;
}

void gar_unmap(void* gar, size_t length)
{
	switch (gar_identify(gar))
	{
		case gar_anonmap:
			platform_virtualfree(gar, length);
			break;
		case gar_uncompressed:
			platform_unmap(gar, length);
			break;
		default:
			break;
	}
}
