#pragma once

#include <inttypes.h>

struct RenderWindow;

namespace render{
	void initialize(int window_width, int window_height, const char* window_title, bool resizable);
	void shutdown(void);

	bool running(void);
	void close_window(void);

	RenderWindow* get_window(void);

	void begin_render(void);
	void end_render(void);
	void clear(void);
	void set_clear_color(uint8_t r, uint8_t g, uint8_t b);

	void set_window_title(const char* title);

	void viewport(int left_x, int top_y, int width, int height);
	void window_size(int& width, int& height);
	void drawable_size(int& width, int& height);

	float time_now(void);
}