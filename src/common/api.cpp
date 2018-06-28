#include "api.h"

#include "io/file.h"
#include "graphics/renderer.h"

struct api_file_t get_file_api(void){
	struct api_file_t result = {0};

#define API(fn) result.fn = io::file::fn
	API(exists);
	API(size);
	API(write);
	API(append);
	API(read);
#undef API

	return result;
}

/*
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
*/
struct api_graphics_t get_graphics_api(void){
	api_graphics_t result = {0};

#define API(fn) result.fn = render::fn
	API(initialize);
	API(shutdown);
	API(running);
	API(close_window);
	API(get_window);
	API(begin_render);
	API(end_render);
	API(clear);
	API(set_clear_color);
	API(set_window_title);
	API(viewport);
	API(window_size);
	API(drawable_size);
#undef API

	return result;
}

api_common_t get_common_api(void){
	api_common_t result = {0};

	result.file = get_file_api();
	result.graphics = get_graphics_api();

	return result;
}