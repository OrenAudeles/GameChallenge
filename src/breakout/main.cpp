#include <dlfcn.h>
#include <stdio.h>
#include <string.h>

#include "common/io/file.h"
#include "common/event/event.h"
#include "common/event/event_SDL_enum.h"
#include "common/event/event_SDL_type.h"
#include "common/api.h"
#include "common/graphics/render_buffer.h"

#include <unordered_map>
struct{
    void* module;
    api_common_t api;
} challenge = {0};

void load_dynamic_libraries(void){
    challenge.module = dlopen("./libchallenge_common.so", RTLD_NOW);
    typedef api_common_t (*api_get)(void);
    challenge.api = ((api_get)dlsym(challenge.module, "get_common_api"))();
}
void unload_dynamic_libraries(void){
    dlclose(challenge.module);
    challenge.module = nullptr;
    challenge.api = {0};
}

EVENT_FN(quit){
    challenge.api.graphics.close_window();
}
EVENT_FN(key_down){
	SDL_Event* ev = (SDL_Event*)data;

	switch(ev->key.keysym.sym){
		case SDLK_ESCAPE: { EVENT_NAME(quit)(ev); break; }
	}
}

uint32_t load_shader(const char*, const char*);
uint32_t load_texture(const char*);

std::unordered_map<std::string, uv_quad> atlas;

void init_atlas(void){
	atlas.clear();

	atlas["full"]  = {0, 0, 1, 1};
	atlas["glyph"] = {0, 0, 10.f / 512.f, 10.f / 288.f};
	atlas["text"]  = {0, 0, 160.f / 512.f, 160.f / 288.f};
	atlas["brick"] = {160.f / 512.f, 0, 128.f / 512.f, 128.f / 288.f};
	atlas["solid"] = {320.f / 512.f, 0, 128.f / 512.f, 128.f / 288.f};
	atlas["ball"]  = {(512 - 64) / 512.f, 0, 64.f / 512.f, 64.f / 288.f};
	atlas["paddle"]= {0, 160.f / 288.f, 1, 128.f / 288.f};
}

int main(int argc, const char** argv){
	load_dynamic_libraries();

	challenge.api.graphics.initialize(800, 600, "Test", false);
	challenge.api.event.initialize_handler();

	init_atlas();

	// Load shader/texture
	uint32_t shader = load_shader("./resource/shader.vs", "./resource/shader.fs");
	uint32_t texture = load_texture("./resource/breakout.png");

	challenge.api.buffer.initialize(1, 1024, shader, texture);

	challenge.api.event.set_event_handler(SDL_QUIT, EVENT_NAME(quit));
	challenge.api.event.set_event_handler(SDL_KEYDOWN, EVENT_NAME(key_down));

	challenge.api.graphics.set_clear_color(128, 128, 0);
	int width, height;
	uint8_t fg[4] = {255, 0, 0, 255};
	uint8_t bg[4] = {255, 0, 0, 255};
	//uv_quad uv = {0, 0, 1, 1};

	render_glyph glyph;
	glyph.x = glyph.y = 0;

	struct Clock_t{
		float last, now;
		inline Clock_t(): last(0), now(challenge.api.graphics.time_now()){}
		inline float delta(void){ return now - last; }
		inline float delta_tick(void){ last = now; now = challenge.api.graphics.time_now(); return delta(); }
	} clock;

	float accum = 0;
	int frames = 0;

	char fps_buf[80] = "Unknown Frame Time, not enough samples";

	float glyph_uv_patch[] = {0, 0, 160.f / 512.f, 160.f/288.f};

	auto render_single_glyph = [&](render_glyph glyph, uint8_t* fg, uint8_t* bg){
		uv_quad uv = {
			glyph_uv_patch[0] + ((glyph_uv_patch[2] - glyph_uv_patch[0]) / 16.f * (glyph.id % 16)),
			glyph_uv_patch[1] + ((glyph_uv_patch[3] - glyph_uv_patch[1]) / 16.f * (glyph.id / 16)),
			(glyph_uv_patch[2] - glyph_uv_patch[0]) / 16.f,
			(glyph_uv_patch[3] - glyph_uv_patch[1]) / 16.f
		};
		challenge.api.buffer.push_RGBA_glyphs_ex(&glyph, 1, &uv, fg, bg);
	};
	auto render_text = [&](const char* text, int x, int y, int w, int h, uint8_t* fg, uint8_t* bg){
		int len = strlen(text);

		render_glyph _glyph;
		_glyph.x = x;
		_glyph.y = y;
		_glyph.w = w;
		_glyph.h = h;
		for (int i = 0; i < len; ++i){
			_glyph.id = text[i];
			render_single_glyph(_glyph, fg, bg);
			_glyph.x += w;
		}
	};
	auto render_fps = [&](void){
		uint8_t _fg[] = {255, 255, 255, 255};
		uint8_t _bg[] = {0, 0, 0, 255};
		render_text(fps_buf, 0, 0, 10, 20, _fg, _bg);
	};

	auto find_or_fail = [&](const std::string& name){
		uv_quad result = {0};

		auto search = atlas.find(name);
		if (search != atlas.end()){
			result = search->second;
		}

		return result;
	};

	auto render_patch = [&](const std::string& name, int x, int y, int w, int h, uint8_t* fg, uint8_t* bg){
		render_glyph _glyph;
		_glyph.x = x;
		_glyph.y = y;
		_glyph.w = w;
		_glyph.h = h;

		uv_quad quad = find_or_fail(name);

		challenge.api.buffer.push_RGBA_glyphs_ex(&_glyph, 1, &quad, fg, bg);
	};

	while (challenge.api.graphics.running()){
		challenge.api.event.poll_events();
		
		float dT = clock.delta_tick();

		accum += dT;
		++frames;

		if (accum >= 1){
			snprintf(fps_buf, 80, "Avg Frame: %.4f s, %f ms", (1 / (float)frames), (1000) / (float)frames);

			accum -= 1;
			frames = 0;
		}

		challenge.api.graphics.drawable_size(width, height);
		challenge.api.buffer.clear(width, height);

		challenge.api.graphics.begin_render();

		glyph.w = width;
		glyph.h = height;

		render_patch("ball", 0, 0, width, height, fg, bg);
		//challenge.api.buffer.push_RGBA_glyphs_ex(&glyph, 1, &uv, fg, bg);

		// Render FPS text
		render_fps();

		challenge.api.buffer.render();
		
		challenge.api.graphics.end_render();
	}

	challenge.api.shader.destroy_program(shader);
	challenge.api.texture.destroy(texture);

	challenge.api.buffer.shutdown();
	challenge.api.event.shutdown_handler();
	challenge.api.graphics.shutdown();

	unload_dynamic_libraries();
	return 0;
}


uint32_t load_shader(const char* vpath, const char* fpath){
	uint32_t result = 0;

	uint32_t vsz = challenge.api.file.size(vpath);
	uint32_t fsz = challenge.api.file.size(fpath);
	
	uint8_t *store = new uint8_t[vsz + fsz];
	void* vdata = store;
	void* fdata = store + vsz;

	challenge.api.file.read(vpath, vdata, vsz);
	challenge.api.file.read(fpath, fdata, fsz);

	result = challenge.api.shader.create_program(vdata, fdata, 0, vsz, fsz, 0);

	delete[] store;

	return result;
}
uint32_t load_texture(const char* path){
	uint32_t result = 0;

	uint32_t tsz = challenge.api.file.size(path);
	uint8_t *store = new uint8_t[tsz];
	void* tdata = store;

	challenge.api.file.read(path, tdata, tsz);
	result = challenge.api.texture.create_alpha(tdata, tsz);
	
	delete[] store;

	return result;
}
