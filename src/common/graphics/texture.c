#include "texture.h"
#include "image.h"

#include <GL/glew.h>

typedef struct {
	uint32_t width, height;
	uint32_t format_internal, format_image;

	uint32_t wrap_s, wrap_t;
	uint32_t filter_min, filter_mag;
} tex_info_t;

static tex_info_t default_texture_data(uint32_t *id){
	glGenTextures(1, id);
	tex_info_t ret = {
		.width = 0,
		.height = 0,
		.format_internal = GL_RGB,
		.format_image = GL_RGB,
		.wrap_s = GL_REPEAT,
		.wrap_t = GL_REPEAT,
		.filter_min = GL_NEAREST,
		.filter_mag = GL_NEAREST
	};

	return ret;
}

uint32_t texture_create(void *tex_data, uint32_t tex_len){
	uint32_t ret = 0;

	tex_info_t info = default_texture_data(&ret);

	uint32_t img = image_load_lump(tex_data, tex_len);

	image_data_t data = image_data(img);

	info.width = data.width;
	info.height = data.height;

	unsigned char* image = data.data;

	glBindTexture(GL_TEXTURE_2D, ret);
	glTexImage2D(GL_TEXTURE_2D,
		0, info.format_internal, info.width, info.height,
		0, info.format_image, GL_UNSIGNED_BYTE, image);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, info.wrap_s);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, info.wrap_t);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, info.filter_min);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, info.filter_mag);

	glBindTexture(GL_TEXTURE_2D, 0);

	image_free(img);

	return ret;
}
uint32_t texture_create_alpha(void *tex_data, uint32_t tex_len){
	uint32_t ret = 0;

	tex_info_t info = default_texture_data(&ret);
	info.format_internal = GL_RGBA;
	info.format_image = GL_RGBA;

	uint32_t img = image_load_lump(tex_data, tex_len);
	image_data_t data = image_data(img);

	info.width = data.width;
	info.height = data.height;

	unsigned char* image = data.data;

	glBindTexture(GL_TEXTURE_2D, ret);
	glTexImage2D(GL_TEXTURE_2D,
		0, info.format_internal, info.width, info.height,
		0, info.format_image, GL_UNSIGNED_BYTE, image);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, info.wrap_s);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, info.wrap_t);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, info.filter_min);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, info.filter_mag);

	glBindTexture(GL_TEXTURE_2D, 0);
	image_free(img);

	return ret;
}

void texture_destroy(uint32_t tex){
	glDeleteTextures(1, &tex);
}

void texture_bind(const uint32_t tex){
	glBindTexture(GL_TEXTURE_2D, tex);
}