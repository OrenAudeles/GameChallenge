#ifndef H_COMMON_API_H
#define H_COMMON_API_H

#pragma once
#include <inttypes.h>

struct RenderWindow;
struct render_glyph;
struct render_clip;
struct uv_quad;

struct api_file_t{
	bool     (*exists)(const char* path);
	uint32_t (*size)  (const char* path);
	bool     (*write) (const char* path, void* data, uint32_t bytes);
	bool     (*append)(const char* path, void* data, uint32_t bytes);
	uint32_t (*read)  (const char* path, void* store, uint32_t bytes);

	uint32_t (*find)(const char* root, const char* ext, void* store, uint32_t store_bytes, uint32_t& store_bytes_used);
	uint32_t (*find_recursive)(const char* root, const char* ext, void* store, uint32_t store_bytes, uint32_t& store_bytes_used);
};
struct api_graphics_t{
	void          (*initialize)      (int width, int height, const char* title, bool resizable);
	void          (*shutdown)        (void);
	bool          (*running)         (void);
	void          (*close_window)    (void);
	RenderWindow* (*get_window)      (void);
	void          (*begin_render)    (void);
	void          (*end_render)      (void);
	void          (*clear)           (void);
	void          (*set_clear_color) (uint8_t r, uint8_t g, uint8_t b);
	void          (*set_window_title)(const char* title);
	void          (*viewport)        (int left_x, int top_y, int width, int height);
	void          (*window_size)     (int& width, int& height);
	void          (*drawable_size)   (int& width, int& height);
	float         (*time_now)        (void);
};

struct api_event_t{
	void (*initialize_handler)(void);
	void (*shutdown_handler)(void);

	void (*set_event_handler)(int event_id, void (*ev_function)(void*));
	void (*call_event_handler)(int event_id, void* event_data);

	void (*poll_events)(void);
	void (*wait_events)(void);
	void (*wait_events_timeout)(float timeout);
};

struct api_shader_t{
	uint32_t (*create_program)(void* vsource, void* fsource, void* gsource, int vlen, int flen, int glen);
	void (*destroy_program)(uint32_t prog);
	void (*use_program)(uint32_t prog);

	void (*set_bool)(uint32_t prog, const char* name, const int value);
	void (*set_int)(uint32_t prog, const char* name, const int32_t value);
	void (*set_float)(uint32_t prog, const char* name, const float value);

	void (*set_vec2)(uint32_t prog, const char* name, const float x, const float y);
	void (*set_vec3)(uint32_t prog, const char* name, const float x, const float y, const float z);
	void (*set_vec4)(uint32_t prog, const char* name, const float x, const float y, const float z, const float w);

	void (*set_vec2v)(uint32_t prog, const char* name, const float* value);
	void (*set_vec3v)(uint32_t prog, const char* name, const float* value);
	void (*set_vec4v)(uint32_t prog, const char* name, const float* value);

	void (*set_mat2)(uint32_t prog, const char* name, const float* value);
	void (*set_mat3)(uint32_t prog, const char* name, const float* value);
	void (*set_mat4)(uint32_t prog, const char* name, const float* value);
};
struct api_texture_t{
	uint32_t (*create)(void* tex_data, uint32_t tex_len);
	uint32_t (*create_alpha)(void* tex_data, uint32_t tex_len);

	void (*destroy)(uint32_t tex);
	void (*bind)(const uint32_t tex);
};

struct api_render_buffer_t{
	void (*initialize)(uint8_t layers, uint32_t max_glyphs, uint32_t shader, uint32_t texture);
	void (*shutdown)(void);
	void (*clear)(uint16_t width, uint16_t height);
	void (*render)(void);
	void (*set_layer)(uint8_t layer);

	void (*push_clip)(render_clip& clip);
	void (*push_refine_clip)(render_clip& clip);
	void (*pop_clip)(void);

	const render_clip& (*current_clip)(void);

	void (*push_glyphs)(render_glyph* glyph, uint32_t count);
	void (*push_alpha_glyphs)(render_glyph* glyph, uint32_t count, uint8_t fg_alpha, uint8_t bg_alpha);
	void (*push_RGBA_glyphs)(render_glyph* glyph, uint32_t count, uint8_t* fg, uint8_t* bg);

	void (*push_RGBA_glyphs_ex)(render_glyph* glyphs, uint32_t count, uv_quad* uv, uint8_t* fg, uint8_t* bg);
};

struct api_common_t{
	api_file_t          file;
    api_graphics_t      graphics;
    api_event_t         event;
    api_shader_t        shader;
    api_texture_t       texture;
    api_render_buffer_t buffer;
};

extern "C" api_common_t get_common_api(void);

#endif
