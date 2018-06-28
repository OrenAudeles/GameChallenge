#ifndef H_COMMON_API_H
#define H_COMMON_API_H

#pragma once
#include <inttypes.h>

struct RenderWindow;

struct api_file_t{
	bool     (*exists)(const char* path);
	uint32_t (*size)  (const char* path);
	bool     (*write) (const char* path, void* data, uint32_t bytes);
	bool     (*append)(const char* path, void* data, uint32_t bytes);
	uint32_t (*read)  (const char* path, void* store, uint32_t bytes);
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

struct api_common_t{
	api_file_t     file;
	api_graphics_t graphics;
	api_event_t    event;
};

extern "C" api_common_t get_common_api(void);

#endif