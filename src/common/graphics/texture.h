#ifndef H_TEXTURE_H
#define H_TEXTURE_H

#include <inttypes.h>

#ifdef __cplusplus
#define EXTERN extern "C" 
#else
#define EXTERN
#endif

EXTERN uint32_t texture_create(void *tex_data, uint32_t tex_len);
EXTERN uint32_t texture_create_alpha(void *tex_data, uint32_t tex_len);

EXTERN void texture_destroy(uint32_t tex);

EXTERN void texture_bind(const uint32_t tex);

#undef EXTERN
#endif