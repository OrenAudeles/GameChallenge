#ifndef H_SHADER_H
#define H_SHADER_H

#include <inttypes.h>

#ifdef __cplusplus
#define EXTERN extern "C" 
#else
#define EXTERN
#endif

EXTERN uint32_t shader_create_program(void* vsource, void* fsource, void* gsource, int32_t vlen, int32_t flen, int32_t glen);
EXTERN void shader_destroy_program(uint32_t prog);

EXTERN void shader_use_program(uint32_t prog);

EXTERN void shader_set_bool(uint32_t prog, const char* name, const int value);
EXTERN void shader_set_int(uint32_t prog, const char* name, const int32_t value);
EXTERN void shader_set_float(uint32_t prog, const char* name, const float value);

EXTERN void shader_set_vec2(uint32_t prog, const char* name, const float x, const float y);
EXTERN void shader_set_vec3(uint32_t prog, const char* name, const float x, const float y, const float z);
EXTERN void shader_set_vec4(uint32_t prog, const char* name, const float x, const float y, const float z, const float w);

EXTERN void shader_set_vec2v(uint32_t prog, const char* name, const float* value);
EXTERN void shader_set_vec3v(uint32_t prog, const char* name, const float* value);
EXTERN void shader_set_vec4v(uint32_t prog, const char* name, const float* value);

EXTERN void shader_set_mat2(uint32_t prog, const char* name, const float* value);
EXTERN void shader_set_mat3(uint32_t prog, const char* name, const float* value);
EXTERN void shader_set_mat4(uint32_t prog, const char* name, const float* value);

#undef EXTERN
#endif