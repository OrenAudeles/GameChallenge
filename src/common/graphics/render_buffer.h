#ifndef H_RENDER_BUFFER_H
#define H_RENDER_BUFFER_H

#pragma once

#include <inttypes.h>

struct render_glyph{
	// Glyph ID to render
	uint32_t id;
	// Screen Position/Dimension of Glyph
	int16_t x, y;
	uint16_t w, h;
	// Glyph Color Codes
	uint8_t bg, fg;
};
struct render_clip{
	// Screen Position/Dimension of Clipping Rectangle
	int16_t x, y;
	uint16_t w, h;
};

struct uv_quad{
	float u, v, du, dv;
};

namespace render{ namespace buffer {
	void initialize(uint8_t layers, uint32_t max_glyphs, uint32_t shader, uint32_t texture);
	void shutdown(void);

	void clear(uint16_t width, uint16_t height);
	void render(void);
	void set_layer(uint8_t layer);

	void push_clip(render_clip& clip);
	// Clips input against current top clipping rectangle prior to insertion
	void push_refine_clip(render_clip& clip);
	void pop_clip(void);

	const render_clip& current_clip(void);

	// Currently all but RGBA glyphs just make the glyphs set colors. No colormap is set
	void push_glyphs(render_glyph* glyph, uint32_t count);

	void push_alpha_glyphs(render_glyph* glyph, uint32_t count, uint8_t fg_alpha, uint8_t bg_alpha);

	// Overrides color coding
	void push_RGBA_glyphs(render_glyph* glyph, uint32_t count, uint8_t* fg, uint8_t* bg);

	// Overrides the glyph-id uv data
	void push_RGBA_glyphs_ex(render_glyph* glyph, uint32_t count, uv_quad* uv, uint8_t* fg, uint8_t* bg);
}}

#endif