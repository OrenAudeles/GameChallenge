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
#include <vector>
#include <cmath>
#include <algorithm>

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

struct PaddleState{
	uint8_t dir_left : 1;
	uint8_t dir_right: 1;
	uint8_t rball    : 1;
} paddle_state = {0};

EVENT_FN(quit){
    challenge.api.graphics.close_window();
}
EVENT_FN(key_down){
	SDL_Event* ev = (SDL_Event*)data;

	switch(ev->key.keysym.sym){
		case SDLK_ESCAPE: { EVENT_NAME(quit)(ev); break; }
		case SDLK_SPACE:  { paddle_state.rball = 1; break; }
		case SDLK_LEFT:   { paddle_state.dir_left = 1; break; }
		case SDLK_RIGHT:  { paddle_state.dir_right = 1; break; }
	}
}
EVENT_FN(key_up){
	SDL_Event* ev = (SDL_Event*)data;

	switch(ev->key.keysym.sym){
		case SDLK_SPACE:{ paddle_state.rball = 0; break; }
		case SDLK_LEFT: { paddle_state.dir_left = 0; break; }
		case SDLK_RIGHT:{ paddle_state.dir_right = 0; break; }
	}
}

uint32_t load_shader(const char*, const char*);
uint32_t load_texture(const char*);

std::unordered_map<std::string, uv_quad> atlas;

void init_atlas(void){
	atlas["full"]  = {0, 0, 1, 1};
	atlas["glyph"] = {0, 0, 10.f / 512.f, 10.f / 288.f};
	atlas["text"]  = {0, 0, 160.f / 512.f, 160.f / 288.f};
	atlas["brick"] = {160.f / 512.f, 0, 128.f / 512.f, 128.f / 288.f};
	atlas["solid"] = {288.f / 512.f, 0, 128.f / 512.f, 128.f / 288.f};
	atlas["ball"]  = {(512 - 64) / 512.f, 0, 64.f / 512.f, 64.f / 288.f};
	atlas["paddle"]= {0, 160.f / 288.f, 1, 128.f / 288.f};
}

void dump_atlas(const char* filename){
	// 16 bytes per ID, 16 bytes per UV
	struct atlas_data{
		uv_quad uv;
		char id[16];
	};

	int count = atlas.size();
	atlas_data *dump = new atlas_data[count];

	int ndx = 0;
	for (auto pair : atlas){
		dump[ndx].uv = pair.second;
		snprintf(dump[ndx].id, 16, "%s", pair.first.c_str());
		++ndx;
	}

	char filename_buf[80] = {0};
	snprintf(filename_buf, 80, "./resource/%s", filename);

	challenge.api.file.write(filename_buf, &count, sizeof(int));
	challenge.api.file.append(filename_buf, dump, count * sizeof(atlas_data));
	delete[] dump;
}
void load_atlas_from_file(const char* filename){
	atlas.clear();
	// 16 bytes per ID, 16 bytes per UV
	struct atlas_data{
		uv_quad uv;
		char id[16];
	};

	char filename_buf[80] = {0};
	snprintf(filename_buf, 80, "./resource/%s", filename);

	//auto sz = 
		challenge.api.file.size(filename_buf);
	uint8_t dbuf[4096];

	// File is either empty, or does not exist. Gotta populate it
	// Once atlas is generated this won't get called again, and
	// can be removed later.
	// if (sz == 0){
	// 	init_atlas();
	// 	dump_atlas(filename);
	// }
	// else{
		challenge.api.file.read(filename_buf, dbuf, 4096);
		int* count_p = (int*)dbuf;

		atlas_data* data_p = (atlas_data*)(count_p + 1);

		for (int i = 0; i < *count_p; ++i){
			printf("[Atlas Loading] %*.*s <%f, %f, %f, %f>\n", 16, 16,
				data_p[i].id, data_p[i].uv.u, data_p[i].uv.v, data_p[i].uv.du, data_p[i].uv.dv);
			atlas[data_p[i].id] = data_p[i].uv;
		}
	// }
}

uv_quad find_or_fail(const std::string& name){
	uv_quad result = {0};

	auto search = atlas.find(name);
	if (search != atlas.end()){
		result = search->second;
	}

	return result;
}

const uint8_t level[] = {
	5, 5,
	1, 1, 2, 1, 1,
	3, 1, 3, 1, 3,
	2, 2, 2, 2, 2,
	2, 1, 2, 1, 2,
	3, 3, 3, 3, 3,
};

struct Box2D{
	float x, y, hw, hh;
};

struct Brick{
	uint8_t type;
	Box2D box;
};
std::vector<Brick> bricks;
int num_non_solid = 0;
int lives = 0;

void init_level(float width, float brick_region_height){
	bricks.clear();
	lives = 3;
	num_non_solid = 0;

	uint8_t bw = level[0];
	uint8_t bh = level[1];

	float brick_width = width / (float)bw;
	float brick_height = brick_region_height / (float)bh;

	float hw = brick_width * 0.5f;
	float hh = brick_height * 0.5f;

	Box2D box;
	box.x = hw;
	box.y = hh;
	box.hw = hw;
	box.hh = hh;

	const uint8_t *index = &level[2];
	for (uint8_t y = 0; y < bh; ++y){
		for (uint8_t x = 0; x < bw; ++x){
			uint8_t t = index[x + y * bw];
			if (t > 0){
				Brick b;
				b.type = t - 1;
				b.box = box;

				bricks.push_back(b);

				num_non_solid += (b.type > 0);
			}
			box.x += brick_width;
		}
		box.y += brick_height;
		box.x = hw;
	}
}

struct Ball{
	float x, y, r;
	float vx, vy;
	bool stuck;
};

Box2D paddle;
Ball  ball;
float paddle_speed = 1.f;
float ball_speed = 0.5f;

struct Collision{
	bool result;
	uint8_t primary_direction;
	float penetration[2];
};
enum PenDir{
	PEN_UP = 1,
	PEN_RIGHT = 2,
	PEN_DOWN = 3,
	PEN_LEFT = 4
};

inline float clamp(float v, float min, float max){
	return std::max(min, std::min(max, v));
}
inline float dot2(const float* a, const float* b){
	return a[0] * b[0] + a[1] * b[1];
}
inline float* normalize2(const float* __restrict a, float* __restrict out){
	float inv_mag = 1.f / std::sqrt(dot2(a, a));
	out[0] = a[0]*inv_mag;
	out[1] = a[1]*inv_mag;

	return out;
}

inline PenDir get_primary_direction(const float* diff){
	const float compass[8] = {
		 0,  1, // UP
		 1,  0, // RIGHT
		 0, -1, // DOWN
		-1,  0  // LEFT
	};

	float norm_res[2] = {0};
	float max = 0;

	int best = -1;
	for (int i = 0; i < 4; ++i){
		float dot = dot2(normalize2(diff, norm_res), &compass[i*2]);
		if (dot > max){
			max = dot;
			best = i;
		}
	}

	return (PenDir)(best + 1);
}
const Collision check_ball_collision(const Box2D& box){
	float diff[2] = {
		ball.x - box.x,
		ball.y - box.y
	};

	float clamped[2] = {
		clamp(diff[0], -box.hw, box.hw),
		clamp(diff[1], -box.hh, box.hh)
	};

	float closest[2] = {
		box.x + clamped[0],
		box.y + clamped[1]
	};

	// Vector between circle center and closest point on box
	diff[0] = closest[0] - ball.x;
	diff[1] = closest[1] - ball.y;

	// squaremag
	float mag2 = diff[0] * diff[0] + diff[1] * diff[1];
	Collision result;
	result.result = (mag2 <= (ball.r * ball.r));

	if (result.result){
		result.primary_direction = get_primary_direction(diff);
		result.penetration[0] = diff[0];
		result.penetration[1] = diff[1];
	}
	return result;
}

void collide_ball_bricks(void){
	std::vector<int> collision_index;

	const float ball_vel[2] = {
		ball.vx > 0 ? ball.vx : -ball.vx,
		ball.vy > 0 ? ball.vy : -ball.vy
	};

	// Build list of bricks being collided with
	for (size_t i = 0; i < bricks.size(); ++i){
		auto col = check_ball_collision(bricks[i].box);

		if (col.result){
			if (bricks[i].type > 0){
				collision_index.push_back(i);
			}
			// Reflection
			PenDir dir = (PenDir)col.primary_direction;
			float *diff_vector = col.penetration;

			float pen[2] = {
				ball.r - (diff_vector[0] > 0 ? diff_vector[0] : -diff_vector[0]),
				ball.r - (diff_vector[1] > 0 ? diff_vector[1] : -diff_vector[1])
			};

			float *ppen = pen;
			float pos_mul = 1;
			float tp = 0, tv = 0;
			float *bpos = &tp, *bvel = &tv;
			int absindex = 0;

			switch(dir){
				case PEN_UP:{
					ppen = &pen[1];
					bpos = &ball.y;
					bvel = &ball.vy;
					pos_mul = -1;
					absindex = 1;
					break;
				}
				case PEN_RIGHT:{
					ppen = &pen[0];
					bpos = &ball.x;
					bvel = &ball.vx;
					pos_mul = -1;
					absindex = 0;
					break;
				}
				case PEN_DOWN:{
					ppen = &pen[1];
					bpos = &ball.y;
					bvel = &ball.vy;
					pos_mul = 1;
					absindex = 1;
					break;
				}
				case PEN_LEFT:{
					ppen = &pen[0];
					bpos = &ball.x;
					bvel = &ball.vx;
					pos_mul = 1;
					absindex = 0;
					break;
				}
			}
			(*bpos) += (pos_mul * (*ppen));
			(*bvel) = (pos_mul) * ball_vel[absindex];
		}
	}

	// Remove from back to front
	for (int i = collision_index.size() - 1; i >= 0; --i){
		bricks[collision_index[i]] = bricks[bricks.size() - 1];
		bricks.pop_back();
	}
	num_non_solid -= collision_index.size();
}

void reset_ball_and_paddle(float width, float height){
	paddle.x = width * 0.5f;
	paddle.y = height * 0.975f;
	paddle.hw = width * 0.1f;
	paddle.hh = height * 0.025f;

	ball.x = paddle.x + (paddle.hw * 0.5f);
	ball.r = 16;
	ball.y = paddle.y - (paddle.hh + ball.r);
	ball.stuck = true;

	paddle_speed = width;
	ball_speed = width * 0.5f;
}

void move_paddle(float width, float height, float dT){
	float dspeed = paddle_speed * dT * ((int)paddle_state.dir_right - (int)paddle_state.dir_left);

	float nx = paddle.x + dspeed;
	
	if (nx + paddle.hw > width){
		nx = width - paddle.hw;
	}
	if (nx - paddle.hw < 0){
		nx = paddle.hw;
	}

	if (ball.stuck){
		ball.x += nx - paddle.x;
	}
	paddle.x = nx;
}
void move_ball(float width, float height, float dT){
	if (!ball.stuck){
		float dspeed = ball_speed * dT;

		float dx = dspeed * ball.vx;
		float dy = dspeed * ball.vy;

		float nx = ball.x + dx;
		float ny = ball.y + dy;

		float left = nx - ball.r;
		float right= nx + ball.r;
		float up   = ny - ball.r;
		float down = ny + ball.r;

		// Collide against walls
		if (left <= 0){
			ball.vx *= -1;
			nx = ball.r;
		}
		if (right >= width){
			ball.vx *= -1;
			nx = width - ball.r;
		}
		if (up <= 0){
			ball.vy *= -1;
			ny = ball.r;
		}
		if (up >= height){
			// reset ball
			reset_ball_and_paddle(width, height);
			--lives;
			nx = ball.x;
			ny = ball.y;
		}
		// Collide against paddle (if traveling DOWN)
		if (dy > 0){
			bool in_x = 
				((left  <= (paddle.x + paddle.hw)) &
				 (right >= (paddle.x - paddle.hw)));
			bool in_y =
				((up    <= (paddle.y + paddle.hh)) &
				 (down  >= (paddle.y - paddle.hh)));

			if (in_x & in_y){
				//ball.vy *= -1;
				float rx = ball.x - paddle.x;
				float ry = ball.y - paddle.y;

				// Vector magnitude
				float mag = sqrtf(rx * rx + ry * ry);
				// Normalized
				float nx = rx / mag;
				float ny = ry / mag;

				ball.vx = nx;
				ball.vy = ny;
			}
		}

		ball.x = nx;
		ball.y = ny;
	}
	else if (paddle_state.rball){
		ball.stuck = false;

		// Calculate direction to fire ball off towards
		// - rel position of Ball to Paddle
		float rx = ball.x - paddle.x;
		float ry = ball.y - paddle.y;

		// Vector magnitude
		float mag = sqrtf(rx * rx + ry * ry);
		// Normalized
		float nx = rx / mag;
		float ny = ry / mag;

		ball.vx = nx;
		ball.vy = ny;
	}
}
void render_paddle(void){
	uint8_t paddle_col[] = {0, 0, 0, 0};

	static auto quad = find_or_fail("paddle");
	render_glyph glyph;
	glyph.x = (paddle.x - paddle.hw);
	glyph.y = (paddle.y - paddle.hh);
	glyph.w = (2 * paddle.hw);
	glyph.h = (2 * paddle.hh);

	challenge.api.buffer.push_RGBA_glyphs_ex(&glyph, 1, &quad, paddle_col, paddle_col);
}
void render_ball(void){
	uint8_t ball_fg[] = {255, 0, 0, 255};
	uint8_t ball_bg[] = {0, 0, 0, 255};

	static auto quad = find_or_fail("ball");
	render_glyph glyph;
	glyph.x = (ball.x - ball.r);
	glyph.y = (ball.y - ball.r);
	glyph.w = (2 * ball.r);
	glyph.h = (2 * ball.r);

	challenge.api.buffer.push_RGBA_glyphs_ex(&glyph, 1, &quad, ball_fg, ball_bg);
}
void render_bricks(void){
	static auto solid_quad = find_or_fail("solid");
	static auto brick_quad = find_or_fail("brick");

	render_glyph glyph;
	uint8_t bg[]       = {0, 0, 0, 255};
	uint8_t solid_fg[] = {0, 0, 0, 255};
	uint8_t brick_fg[][4] = {
		{0, 255, 0, 255},
		{0, 0, 255, 255}
	};

	uint8_t* fg = nullptr;
	uv_quad* quad = nullptr;

	for (Brick b : bricks){
		if (b.type == 0){
			fg = solid_fg;
			quad = &solid_quad;
		}
		else{
			fg = brick_fg[b.type - 1];
			quad = &brick_quad;
		}

		glyph.x = b.box.x - b.box.hw;
		glyph.y = b.box.y - b.box.hh;
		glyph.w = 2 * b.box.hw;
		glyph.h = 2 * b.box.hh;

		challenge.api.buffer.push_RGBA_glyphs_ex(&glyph, 1, quad, fg, bg);
	}
}

int test_win_lose(void){
	return (num_non_solid == 0) - (lives == -1);
}

int main(int argc, const char** argv){
	load_dynamic_libraries();

	int width = 800, height = 600;
	challenge.api.graphics.initialize(width, height, "Test", false);
	challenge.api.event.initialize_handler();

	// Load shader/texture
	uint32_t shader = load_shader("./resource/shader.vs", "./resource/shader.fs");
	uint32_t texture = load_texture("./resource/breakout.png");
	load_atlas_from_file("breakout.atlas");

	challenge.api.buffer.initialize(1, 1024, shader, texture);

	challenge.api.event.set_event_handler(SDL_QUIT, EVENT_NAME(quit));
	challenge.api.event.set_event_handler(SDL_KEYDOWN, EVENT_NAME(key_down));
	challenge.api.event.set_event_handler(SDL_KEYUP, EVENT_NAME(key_up));

	challenge.api.graphics.set_clear_color(128, 128, 0);
	//uint8_t fg[4] = {255, 0, 0, 255};
	//uint8_t bg[4] = {255, 0, 0, 255};
	
	struct Clock_t{
		float last, now;
		inline Clock_t(): last(0), now(challenge.api.graphics.time_now()){}
		inline float delta(void){ return now - last; }
		inline float delta_tick(void){ last = now; now = challenge.api.graphics.time_now(); return delta(); }
	} clock;

	float accum = 0;
	int frames = 0;

	char fps_buf[80] = "Unknown Frame Time, not enough samples";

	auto render_single_glyph = [&](render_glyph rglyph, uint8_t* fg, uint8_t* bg){
		static uv_quad text = find_or_fail("text");
		static uv_quad glyph = find_or_fail("glyph");

		float ou = (rglyph.id % 16) * glyph.du;
		float ov = (rglyph.id / 16) * glyph.dv;
		
		uv_quad uv = {
			text.u + ou,
			text.v + ov,
			glyph.du, glyph.dv
		};
		challenge.api.buffer.push_RGBA_glyphs_ex(&rglyph, 1, &uv, fg, bg);
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
	auto render_lives = [&](void){
		uint8_t _fg[] = {255, 255, 255, 255};
		uint8_t _bg[] = {0, 0, 0, 255};
		char life_buf[80] = {0};

		int len = snprintf(life_buf, 80, "Lives Remaining: %d", lives);
		render_text(life_buf, width - (10 * len), 0, 10, 20, _fg, _bg);
	};
	auto render_win = [&](void){
		uint8_t _fg[] = {255, 255, 255, 255};
		uint8_t _bg[] = {0, 0, 0, 255};
		
		render_text("You Win!", 0, 0, width / 7, height, _fg, _bg);
		render_text("Press [ESC] to exit", 0, height - 20, 10, 20, _fg, _bg);
	};
	auto render_lose = [&](void){
		uint8_t _fg[] = {255, 255, 255, 255};
		uint8_t _bg[] = {0, 0, 0, 255};
		
		render_text("You Lose!", 0, 0, width / 8, height, _fg, _bg);
		render_text("Press [ESC] to exit", 0, height - 20, 10, 20, _fg, _bg);
	};

	// auto render_patch = [&](const std::string& name, int x, int y, int w, int h, uint8_t* fg, uint8_t* bg){
	// 	render_glyph _glyph;
	// 	_glyph.x = x;
	// 	_glyph.y = y;
	// 	_glyph.w = w;
	// 	_glyph.h = h;

	// 	uv_quad quad = find_or_fail(name);

	// 	challenge.api.buffer.push_RGBA_glyphs_ex(&_glyph, 1, &quad, fg, bg);
	// };

	reset_ball_and_paddle(width, height);
	init_level(width, height * 0.5f);

	while (challenge.api.graphics.running()){
		challenge.api.event.poll_events();
		challenge.api.graphics.drawable_size(width, height);
		
		float dT = clock.delta_tick();

		accum += dT;
		++frames;

		if (accum >= 1){
			snprintf(fps_buf, 80, "Avg Frame: %.4f s, %f ms", (1 / (float)frames), (1000) / (float)frames);

			accum -= 1;
			frames = 0;
		}


		move_paddle(width, height, dT);
		move_ball(width, height, dT);
		collide_ball_bricks();

		int winlose = test_win_lose();

		challenge.api.buffer.clear(width, height);

		challenge.api.graphics.begin_render();

		//render_patch("full", 0, 0, width, height, fg, bg);
		//render_patch("ball", 32, 32, 16, 16, fg, bg);
		//render_patch("brick", 0, 64, width / 5, height / 10, fg, bg);
		//render_patch("paddle", width / 2 - width / 10, height - height / 20, width / 5, height / 20, fg, fg);
		switch (winlose){
			case -1:{
				render_lose();
				break;
			}
			case 0:{
				render_paddle();
				render_bricks();
				render_ball();
				
				render_lives();
				break;
			}
			case 1:{
				render_win();
				break;
			}
		}


		
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
