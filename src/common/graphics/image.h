#ifndef H_IMAGE_H
#define H_IMAGE_H

#include <inttypes.h>

typedef struct {
	int width, height, bpp;
	void* data;
} image_data_t;

#ifdef __cplusplus
#define EXTERN extern "C" 
#else
#define EXTERN
#endif

EXTERN void         image_initialize(void);

EXTERN uint32_t     image_load_lump(void* data, uint32_t data_bytes);
EXTERN uint32_t     image_load(const char* path);
EXTERN void         image_free(uint32_t img);
EXTERN image_data_t image_data(uint32_t img);

#undef EXTERN
#endif