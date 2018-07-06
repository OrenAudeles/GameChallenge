#include "api.h"

#include "io/file.h"
#include "event/event.h"
#include "memory/free.h"

#include "graphics/image.h"
#include "graphics/renderer.h"
#include "graphics/shader.h"
#include "graphics/texture.h"
#include "graphics/render_buffer.h"

struct api_file_t get_file_api(void){
	struct api_file_t result = {0};

#define API(fn) result.fn = io::file::fn
	API(exists);
	API(size);
	API(write);
	API(append);
	API(read);

	API(find);
	API(find_recursive);
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
	API(time_now);
#undef API

	return result;
}

struct api_image_t get_image_api(void){
	api_image_t result = {0};
#define API(fn) result.fn = image_##fn
	API(initialize);
	API(load);
	API(load_lump);
	API(free);
	API(data);
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

struct api_shader_t get_shader_api(void){
	api_shader_t result = {0};
#define API(fn) result.fn = shader_##fn
	API(create_program);
	API(destroy_program);
	API(use_program);
	API(set_bool);
	API(set_int);
	API(set_float);
	API(set_vec2);
	API(set_vec3);
	API(set_vec4);
	API(set_vec2v);
	API(set_vec3v);
	API(set_vec4v);
	API(set_mat2);
	API(set_mat3);
	API(set_mat4);
#undef API
	return result;
}
struct api_texture_t get_texture_api(void){
	api_texture_t result = {0};
#define API(fn) result.fn = texture_##fn
	API(create);
	API(create_alpha);
	API(destroy);
	API(bind);
#undef API
	return result;
}

struct api_render_buffer_t get_buffer_api(void){
	api_render_buffer_t result = {0};
#define API(fn) result.fn = render::buffer::fn
	API(initialize);
	API(shutdown);
	API(clear);
	API(render);
	API(set_layer);
	API(push_clip);
	API(push_refine_clip);
	API(pop_clip);
	API(current_clip);
	API(push_glyphs);
	API(push_alpha_glyphs);
	API(push_RGBA_glyphs);
	API(push_RGBA_glyphs_ex);
#undef API
	return result;
}

struct api_memory_t get_memory_api(void){
	api_memory_t result = {0};
#define API(fn) result.fn = memory_##fn
	API(free);
#undef API
	return result;
}

api_common_t get_common_api(void){
	api_common_t result = {0};

	result.memory = get_memory_api();
	result.file = get_file_api();
	result.image = get_image_api();
	result.graphics = get_graphics_api();
	result.event = get_event_api();
	result.shader = get_shader_api();
	result.texture = get_texture_api();
	result.buffer = get_buffer_api();

	return result;
}