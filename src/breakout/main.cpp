#include <dlfcn.h>
#include <stdio.h>

#include "common/io/file.h"
#include "common/event/event.h"
#include "common/event/event_SDL_enum.h"
#include "common/api.h"
#include "common/graphics/render_buffer.h"

// #include <chrono>
// #include <thread>

void *challenge_lib = nullptr;
api_common_t challenge_api = {0};

void load_dynamic_libraries(void){
	printf("Attempting to load Challenge Library\n");
	challenge_lib = dlopen("./libchallenge_common.so", RTLD_NOW);

	typedef api_common_t (*api_get)(void);

	api_get get = (api_get)dlsym(challenge_lib, "get_common_api");

	printf("Getting challenge API\n");
	challenge_api = get();

	// printf("Checking for 'breakout' existence\n");
	// bool breakout_exist = challenge_api.file.exists("breakout");
	// printf("-- Breakout Exists? [%c]\n",  breakout_exist ? 'T' : 'F');
	// if (breakout_exist){
	// 	printf("-- %u Bytes\n", challenge_api.file.size("breakout"));
	// }
	printf("Size of API: %zu\n", sizeof(challenge_api));

	printf("[CHALLENGE LIBRARY LOADED]\n");
}
void unload_dynamic_libraries(void){
	dlclose(challenge_lib);
	challenge_lib = nullptr;

	api_common_t tapi = {0};
	challenge_api = tapi;

	printf("[CHALLENGE LIBRARY UNLOADED]\n");
}

EVENT_FN(quit){
	challenge_api.graphics.close_window();
}

uint32_t load_shader(const char*, const char*);
uint32_t load_texture(const char*);

int main(int argc, const char** argv){
	load_dynamic_libraries();

	challenge_api.graphics.initialize(800, 600, "Test", false);
	challenge_api.event.initialize_handler();

	// Load shader/texture
	uint32_t shader = load_shader("./resource/codepage.vs", "./resource/codepage.fs");
	uint32_t texture = load_texture("./resource/codepage.png");

	challenge_api.buffer.initialize(1, 1024, shader, texture);

	challenge_api.event.set_event_handler(SDL_QUIT, EVENT_NAME(quit));

	// {
	// 	using namespace std::this_thread;
	// 	using namespace std::chrono_literals;
	// 	using std::chrono::high_resolution_clock;

	// 	sleep_until(high_resolution_clock::now() + 1s);
	// }
//void (*push_RGBA_glyphs_ex)(render_glyph* glyphs, uint32_t count, float* uv[4], uint8_t* fg, uint8_t* bg);

	challenge_api.graphics.set_clear_color(128, 128, 0);
	int width, height;
	uint8_t fg[4] = {255, 0, 0, 255};
	uint8_t bg[4] = {0, 255, 0, 255};
	uv_quad uv = {0, 0, 1, 1};

	render_glyph glyph;
	glyph.x = glyph.y = 0;

	while (challenge_api.graphics.running()){
		challenge_api.event.poll_events();
		challenge_api.graphics.drawable_size(width, height);
		challenge_api.buffer.clear(width, height);

		challenge_api.graphics.begin_render();

		glyph.w = width;
		glyph.h = height;

		challenge_api.buffer.push_RGBA_glyphs_ex(&glyph, 1, &uv, fg, bg);
		challenge_api.buffer.render();
		
		challenge_api.graphics.end_render();
	}

	challenge_api.shader.destroy_program(shader);
	challenge_api.texture.destroy(texture);

	challenge_api.buffer.shutdown();
	challenge_api.event.shutdown_handler();
	challenge_api.graphics.shutdown();

	unload_dynamic_libraries();
	return 0;
}


uint32_t load_shader(const char* vpath, const char* fpath){
	uint32_t result = 0;

	uint32_t vsz = challenge_api.file.size(vpath);
	uint32_t fsz = challenge_api.file.size(fpath);
	
	uint8_t *store = new uint8_t[vsz + fsz];
	void* vdata = store;
	void* fdata = store + vsz;

	challenge_api.file.read(vpath, vdata, vsz);
	challenge_api.file.read(fpath, fdata, fsz);

	result = challenge_api.shader.create_program(vdata, fdata, 0, vsz, fsz, 0);

	delete[] store;

	return result;
}
uint32_t load_texture(const char* path){
	uint32_t result = 0;

	uint32_t tsz = challenge_api.file.size(path);
	uint8_t *store = new uint8_t[tsz];
	void* tdata = store;

	challenge_api.file.read(path, tdata, tsz);
	result = challenge_api.texture.create_alpha(tdata, tsz);
	
	delete[] store;

	return result;
}