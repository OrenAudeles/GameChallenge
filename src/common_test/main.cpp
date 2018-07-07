#include <dlfcn.h>
#include <stdio.h>
#include <string.h>

#include "common/event/event.h"
#include "common/event/event_SDL_enum.h"
#include "common/event/event_SDL_type.h"
#include "common/api.h"
#include "common/graphics/render_buffer.h"

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

int main(int argc, const char** argv){
	load_dynamic_libraries();

	challenge.api.graphics.initialize(800, 600, "Test", false);
	challenge.api.event.initialize_handler();

	uint32_t shader = 0, texture = 0;
	{
		// Find a vertex shader, and a fragment shader
		// Should only ever be one of each. Doesn't matter
		// what they are named though since we are just searching
		// for the extension
		uint8_t buffer[1024];
		uint32_t buffer_used = 0;
		uint32_t nvs = challenge.api.file.find("./resource", ".vs", buffer, 1024, buffer_used);
		uint32_t vs_buf_used = buffer_used;
		uint32_t nfs = challenge.api.file.find("./resource", ".fs", buffer + vs_buf_used, 1024 - vs_buf_used, buffer_used);

		// Load shader/texture if we have exactly one of each available
		if (nvs == 1 && nfs == 1){
			shader = load_shader((const char*)buffer, (const char*)(buffer + vs_buf_used));
		}
	}
	// hardcoded relative path
	texture = load_texture("./resource/codepage.png");


	challenge.api.buffer.initialize(1, 1024, shader, texture);

	challenge.api.event.set_event_handler(SDL_QUIT, EVENT_NAME(quit));
	challenge.api.event.set_event_handler(SDL_KEYDOWN, EVENT_NAME(key_down));

	challenge.api.graphics.set_clear_color(128, 128, 0);
	int width, height;
	uint8_t fg[4] = {255, 255, 255, 255};
	uint8_t bg[4] = {0, 0, 0, 255};
	uv_quad uv = {0, 0, 1, 1};

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

	float glyph_uv_patch[] = {0, 0, 160.f/160.f, 160.f/160.f};

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

	struct Color4{
		uint8_t r, g, b, a;
	};

	auto lerp = [](float a, float b, float t){
		return a + (b - a) * t;
	};
	auto color_lerp = [&](Color4 a, Color4 b, float t){
		Color4 result = {0};
		struct {
			float r, g, b, a;
		} result_f;

		result_f.r = lerp(a.r, b.r, t);
		result_f.g = lerp(a.g, b.g, t);
		result_f.b = lerp(a.b, b.b, t);
		result_f.a = lerp(a.a, b.a, t);

		result.r = (uint8_t)result_f.r;
		result.g = (uint8_t)result_f.g;
		result.b = (uint8_t)result_f.b;
		result.a = (uint8_t)result_f.a;

		return result;
	};

	auto color_lerp_wheel = [&](Color4 *colors, uint8_t ncol, float ab_time, float curtime){
		int now = curtime / ab_time;
		int next = now + 1;

		return color_lerp(colors[now % ncol], colors[next % ncol], (curtime / ab_time) - now);
	};

	Color4 cols[] = {
		{255, 0, 0, 255},
		{0, 255, 0, 255},
		{0, 0, 255, 255}
	};

	float total_time = 0;

	auto fill_col8 = [](uint8_t* col, Color4 from){
		col[0] = from.r;
		col[1] = from.g;
		col[2] = from.b;
		col[3] = from.a;
	};

	float interval = 1.f / 4.f;

	float framerate_cache[60] = {0};
	int head = 0, tail = 0;

	auto push_frametime = [&](float time){
		framerate_cache[tail] = time;
		tail = (tail + 1) % 60;

		if (tail == head){
			head = (head + 1)%60;
		}
	};

	auto render_frametime = [&](int x, int y, int w, int h, uint8_t* fg, uint8_t* bg){
		/*
		uv_quad uv = {
			glyph_uv_patch[0] + ((glyph_uv_patch[2] - glyph_uv_patch[0]) / 16.f * (glyph.id % 16)),
			glyph_uv_patch[1] + ((glyph_uv_patch[3] - glyph_uv_patch[1]) / 16.f * (glyph.id / 16)),
			(glyph_uv_patch[2] - glyph_uv_patch[0]) / 16.f,
			(glyph_uv_patch[3] - glyph_uv_patch[1]) / 16.f
		};
		challenge.api.buffer.push_RGBA_glyphs_ex(&glyph, 1, &uv, fg, bg);
		*/
		render_glyph glyphs[60] = {0};
		uv_quad uvs[60] = {0};

		uv_quad uv = {
			glyph_uv_patch[0] + ((glyph_uv_patch[2] - glyph_uv_patch[0]) / 16.f * (0 % 16)),
			glyph_uv_patch[1] + ((glyph_uv_patch[3] - glyph_uv_patch[1]) / 16.f * (0 / 16)),
			(glyph_uv_patch[2] - glyph_uv_patch[0]) / 16.f,
			(glyph_uv_patch[3] - glyph_uv_patch[1]) / 16.f
		};

		for (int i = 0; i < 60; ++i){
			uvs[i] = uv;
		}

		int n = 0;
		int gw = w / 60;
		for (int i = head; i < 60; ++i, ++n){
			glyphs[i].x = x + gw * n;
			int dh = (h * 0.5f) * (framerate_cache[i] * 60.f);
			glyphs[i].y = y + h - dh;
			glyphs[i].w = gw;
			glyphs[i].h = dh;
		}
		for (int i = 0; i < head; ++i, ++n){
			glyphs[i].x = x + gw * n;
			int dh = (h * 0.5f) * (framerate_cache[i] * 60.f);
			glyphs[i].y = y + h - dh;
			glyphs[i].w = gw;
			glyphs[i].h = dh;
		}

		challenge.api.buffer.push_RGBA_glyphs_ex(glyphs, 60, uvs, fg, bg);
	};

	while (challenge.api.graphics.running()){
		challenge.api.event.poll_events();
		
		float dT = clock.delta_tick();

		Color4 col = color_lerp_wheel(cols, 3, 2, total_time);
		fill_col8(bg, col);

		accum += dT;
		++frames;
		total_time += dT;

		if (accum >= interval){
			float time = interval / (float)frames;
			snprintf(fps_buf, 80, "Avg Frame: %.4f s, %f ms", time, time * 1000);

			accum -= interval;
			frames = 0;
			//push_frametime(time);
		}
		push_frametime(dT);

		challenge.api.graphics.drawable_size(width, height);
		challenge.api.buffer.clear(width, height);

		challenge.api.graphics.begin_render();

		glyph.w = width;
		glyph.h = height - 80;

		challenge.api.buffer.push_RGBA_glyphs_ex(&glyph, 1, &uv, fg, bg);

		// Render FPS text
		render_fps();
		render_frametime(0, height - 80, width, 80, fg, fg);

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

// Load Vertex/Fragment shader files into memory then pass that data
// into the shader program compiler
uint32_t load_shader(const char* vpath, const char* fpath){
	printf("Loading shader from: V[%s], F[%s]\n", vpath, fpath);
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
// Load texture file into memory, then pass into texture loading to GPU
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
