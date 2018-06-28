#include "api.h"

#include "io/file.h"
#include "graphics/renderer.h"
#include "event/event.h"

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

struct api_event_t get_event_api(void){
	api_event_t result = {0};

#define API(fn) result.fn = event::fn
	API(initialize_handler);
	API(shutdown_handler);
	API(set_event_handler);
	API(call_event_handler);
	API(poll_events);
	API(wait_events);
	API(wait_events_timeout);
#undef API

	return result;
}

api_common_t get_common_api(void){
	api_common_t result = {0};

	result.file = get_file_api();
	result.graphics = get_graphics_api();
	result.event = get_event_api();

	return result;
}