#include <stdint.h>
#include <GL/gl.h>
#include "image.h"
#include "bfingers.h"

image_texture* image_totexture(uint8_t* data)
{
	bmpfile_header* header = (void*)data + 2;
	bmpfile_infoheader* infoheader = (void*)data + 14;
	uint8_t* bmpdata = data + ltoh32(header->bmp_offset), * flipped;
	image_texture* ret = malloc(sizeof(image_texture));
	long i, width, height;
	
	if (data[0] != 'B' || data[1] != 'M')
		return 0;
	
	width = ltoh32(infoheader->width);
	height = ltoh32(infoheader->height);
	
	if (ltoh32(infoheader->header_sz) < 40 || ltoh16(infoheader->nplanes) > 1 || ltoh16(infoheader->bitspp) != 32 || ltoh32(infoheader->compress_type) != 0
		|| !width || height <= 0)
		return 0;
	
	glGenTextures(1, &ret->texture);
	glBindTexture(GL_TEXTURE_2D, ret->texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	
	if (width > 0)
	{
		unsigned long pitch = width * 4;
		
		flipped = malloc(height * pitch);
		for (i = height * pitch - pitch; i >= 0; i -= pitch, bmpdata += pitch)
			memcpy(flipped + i, bmpdata, pitch);
	}
	
	ret->refs = 1;
	ret->width = width;
	ret->height = height;
	ret->texwidth = width;
	ret->texheight = height;
	ret->ratx = 1.0;
	ret->raty = 1.0;
	
	if (width > 0)
	{
		glTexImage2D(GL_TEXTURE_2D, 0, 4, ret->texwidth, ret->texheight, 0, GL_BGRA, GL_UNSIGNED_BYTE, flipped);
		free(flipped);
	}
	else
		glTexImage2D(GL_TEXTURE_2D, 0, 4, ret->texwidth, ret->texheight, 0, GL_BGRA, GL_UNSIGNED_BYTE, bmpdata);
	
	return ret;
}

void image_drawtexture(image_texture* texture, double x, double y)
{
	glBindTexture(GL_TEXTURE_2D, texture->texture);
	glBegin(GL_QUADS);
		glTexCoord2d(            0,             0); glVertex2d(                 x,                   y);
		glTexCoord2d(texture->ratx,             0); glVertex2d(x + texture->width,                   y);
		glTexCoord2d(texture->ratx, texture->raty); glVertex2d(x + texture->width, y + texture->height);
		glTexCoord2d(            0, texture->raty); glVertex2d(                 x, y + texture->height);
	glEnd();
}

void image_drawscale(image_texture* texture, double x, double y, double width, double height)
{
	glBindTexture(GL_TEXTURE_2D, texture->texture);
	glBegin(GL_QUADS);
		glTexCoord2d(            0,             0); glVertex2d(        x,          y);
		glTexCoord2d(texture->ratx,             0); glVertex2d(x + width,          y);
		glTexCoord2d(texture->ratx, texture->raty); glVertex2d(x + width, y + height);
		glTexCoord2d(            0, texture->raty); glVertex2d(        x, y + height);
	glEnd();
}

void image_drawrotate(image_texture* texture, double x, double y, double width, double height, double rotx, double roty, double angle)
{
	glPushMatrix();
	glTranslated(x + rotx, y + roty, 0);
	glRotated(angle, 0, 0, 1.0);
	glTranslated(-rotx, -roty, 0);
	glBindTexture(GL_TEXTURE_2D, texture->texture);
	glBegin(GL_QUADS);
		glTexCoord2d(            0,             0); glVertex2d(    0,      0);
		glTexCoord2d(texture->ratx,             0); glVertex2d(width,      0);
		glTexCoord2d(texture->ratx, texture->raty); glVertex2d(width, height);
		glTexCoord2d(            0, texture->raty); glVertex2d(    0, height);
	glEnd();
	glPopMatrix();
}

void image_deletetexture(image_texture* texture)
{
	if (!--texture->refs)
	{
		glDeleteTextures(1, &texture->texture);
		free(texture);
	}
}
