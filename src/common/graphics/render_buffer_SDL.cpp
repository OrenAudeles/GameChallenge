#include "render_buffer.h"
#include "renderer.h"

#include "shader.h"
#include "texture.h"

#include <new>
#include <vector>
#include <string.h>

#include <GL/glew.h>

#include <stdio.h>

struct render_vertex{
	float position[2];
	float uv[2];
	uint8_t color0[4];
	uint8_t color1[4];
};

struct quad_t{
	// First of 4 vertices
	uint32_t vertex_ndx;
	// First of 6 elements
	uint32_t element_ndx;
};

struct render_layer{
	std::vector<quad_t> quad;
};

class SDL_RenderBuffer{
public:
	SDL_RenderBuffer(uint8_t nlayers, uint32_t vertices, uint32_t elements):
		layers(0), vertex_store(0), element_store(0),
		vSize(0), vCapacity(vertices), eSize(0), eCapacity(elements),
		VAO(0), VBO(0), EBO(0), TID(0), SID(0), width(0), height(0),
		_clip_top(0), _layer_count(nlayers), cLayer(0)
	{
		layers = new render_layer[nlayers];
		_clip_rect[0] = render_clip{0};

		vertex_store = new render_vertex[vertices];
		element_store = new uint32_t[elements];
	}
	~SDL_RenderBuffer(void){
		delete[] layers;
		delete[] vertex_store;
		delete[] element_store;

		layers = nullptr;
		vertex_store = nullptr;
		element_store = nullptr;

		_layer_count = 0;

		glDeleteVertexArrays(1, &VAO);
		glDeleteBuffers(1, &VBO);
		glDeleteBuffers(1, &EBO);
	}

	render_layer *layers;
	render_vertex *vertex_store;
	uint32_t      *element_store;

	render_clip _clip_rect[32];
	uint32_t vSize, vCapacity;
	uint32_t eSize, eCapacity;

	uint32_t VAO, VBO, EBO;
	uint32_t TID, SID;

	uint16_t width, height;
	
	uint8_t _clip_top;
	uint8_t _layer_count;
	uint8_t cLayer;
};

namespace {
	static uint8_t buffer_renderbuffer[sizeof(SDL_RenderBuffer)];
	SDL_RenderBuffer* render_buffer = nullptr;

	render_vertex* alloc_vertices(uint32_t count){
		render_vertex* result = render_buffer->vertex_store + render_buffer->vSize;
		render_buffer->vSize += count;

		return result;
	}
	uint32_t* alloc_elements(uint32_t count){
		uint32_t* result = render_buffer->element_store + render_buffer->eSize;
		render_buffer->eSize += count;

		return result;
	}

	inline render_vertex* make_vertex(render_vertex* vert, float x, float y, float u, float v, uint8_t* bg, uint8_t* fg){
		vert->position[0] = x; vert->position[1] = y;
		vert->uv[0] = u; vert->uv[1] = v;
		vert->color0[0] = bg[0]; vert->color0[1] = bg[1]; vert->color0[2] = bg[2]; vert->color0[3] = bg[3];
		vert->color1[0] = fg[0]; vert->color1[1] = fg[1]; vert->color1[2] = fg[2]; vert->color1[3] = fg[3];

		return vert + 1;
	}
	inline uint32_t* make_element(uint32_t* e, uint32_t offset, uint32_t v){
		*e = offset + v;
		return e + 1;
	}
	inline void get_uvs(uint32_t glyph, float* uvs){
		uvs[0] = uvs[1] = uvs[2] = uvs[3] = 0;
		if (glyph <= 255){
			const float du = 1 / 16.f;
			const float dv = 1 / 16.f;

			uvs[2] = du;
			uvs[3] = dv;
			uvs[0] = du * (glyph % 16);
			uvs[1] = dv * (glyph / 16);
		}
	}
	inline void get_color(uint32_t cid, uint8_t* col){
		col[0] = 255; col[1] = col[2] = 0;
	}

	void init_render_data(void){
		uint32_t &VAO = render_buffer->VAO;
		uint32_t &VBO = render_buffer->VBO;
		uint32_t &EBO = render_buffer->EBO;

		// Generate
		glGenVertexArrays(1, &VAO);
		glGenBuffers(1, &VBO);
		glGenBuffers(1, &EBO);

		// Bind
		glBindVertexArray(VAO);
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);

		// Set attrib data
		glEnableVertexAttribArray(0);
		glEnableVertexAttribArray(1);
		glEnableVertexAttribArray(2);
		glEnableVertexAttribArray(3);

		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(render_vertex), (GLvoid*)offsetof(render_vertex, position));
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(render_vertex), (GLvoid*)offsetof(render_vertex, uv));
		glVertexAttribPointer(2, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(render_vertex), (GLvoid*)offsetof(render_vertex, color0));
		glVertexAttribPointer(3, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(render_vertex), (GLvoid*)offsetof(render_vertex, color1));

		glBindVertexArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	}
}

namespace render{ namespace buffer {
	void initialize(uint8_t layers, uint32_t max_glyphs, uint32_t shader, uint32_t texture){
		if (render_buffer != nullptr){ shutdown(); }
		render_buffer = new (buffer_renderbuffer) SDL_RenderBuffer(layers, max_glyphs * 4, max_glyphs * 6);

		render_buffer->SID = shader;
		render_buffer->TID = texture;

		// Initialize rendering data
		init_render_data();

		// Generate atlas?
	}
	void shutdown(void){
		render_buffer->~SDL_RenderBuffer();
		render_buffer = nullptr;
	}

	void clear(uint16_t width, uint16_t height){
		for (uint32_t i = 0; i < render_buffer->_layer_count; ++i){
			render_buffer->layers[i].quad.clear();
		}
		render_buffer->vSize = render_buffer->eSize = 0;
		render_buffer->cLayer = 0;
		render_buffer->_clip_top = 0;

		render_buffer->width = width;
		render_buffer->height = height;
		render_buffer->_clip_rect[0] = {0, 0, width, height};
	}
	void render(void){
		float ortho[4][4] = {
			{ 2, 0, 0, 0},
			{ 0,-2, 0, 0},
			{ 0, 0,-1, 0},
			{-1, 1, 0, 1}
		};
		{
			ortho[0][0] /= (float)render_buffer->width;
			ortho[1][1] /= (float)render_buffer->height;
		}
		glActiveTexture(GL_TEXTURE0);

		shader_use_program(render_buffer->SID);
		texture_bind(render_buffer->TID);

		shader_set_mat4(render_buffer->SID, "projection", &ortho[0][0]);
		shader_set_int(render_buffer->SID, "image", 0);

		// Bind VAO and buffers
		glBindVertexArray(render_buffer->VAO);
		glBindBuffer(GL_ARRAY_BUFFER, render_buffer->VBO);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, render_buffer->EBO);
		{
			glBufferData(GL_ARRAY_BUFFER, render_buffer->vCapacity * sizeof(render_vertex), NULL, GL_STREAM_DRAW);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, render_buffer->eCapacity * sizeof(uint32_t), NULL, GL_STREAM_DRAW);
			
			// Map buffers
			void* vvertices = glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
			void* velements = glMapBuffer(GL_ELEMENT_ARRAY_BUFFER, GL_WRITE_ONLY);

			render_vertex* vertices = (render_vertex*)vvertices;
			uint32_t* elements = (uint32_t*)velements;

			uint32_t nElements = 0;

			// Now draw all of the buffer contents!
			// Do a raw copy of vertices into the buffer
			memcpy(vertices, render_buffer->vertex_store, render_buffer->vSize * sizeof(render_vertex));

			// Copy in quad elements
			for (uint32_t i = 0; i < render_buffer->_layer_count; ++i){
				render_layer* buf = render_buffer->layers + i;
				
				const auto& quad = buf->quad;

				for (const quad_t &q : quad){
					memcpy(elements + nElements, render_buffer->element_store + q.element_ndx, 6 * sizeof(uint32_t));
					nElements += 6;
				}
			}

			// Unmap buffers!
			glUnmapBuffer(GL_ARRAY_BUFFER);
			glUnmapBuffer(GL_ELEMENT_ARRAY_BUFFER);

			// Draw it!
			glDrawElements(GL_TRIANGLES, nElements, GL_UNSIGNED_INT, 0);
		}

		// Unbind buffers
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
		glBindVertexArray(0);
	}
	void set_layer(uint8_t layer){
		render_buffer->cLayer = layer;
	}

	void push_clip(render_clip& clip){
		render_buffer->_clip_rect[++render_buffer->_clip_top] = clip;
	}
	void push_refine_clip(render_clip& clip){
		// clip the clip against current clip_top
		render_clip ctop = render_buffer->_clip_rect[render_buffer->_clip_top];

		// Split ctop into real coordinates {x,y, x1,y1}
		int ctop_vals[] = {ctop.x, ctop.y, ctop.x + ctop.w, ctop.y + ctop.h};
		// split clip into real coordinates {x,y, x1,y1}
		int clip_vals[] = {clip.x, clip.y, clip.x + clip.w, clip.y + clip.h};

		// clip.x must be equal or right of ctop x
		clip_vals[0] = clip_vals[0] < ctop_vals[0] ? ctop_vals[0] : clip_vals[0];
		// same with y
		clip_vals[1] = clip_vals[1] < ctop_vals[1] ? ctop_vals[1] : clip_vals[1];

		// Extants x1,y1 must be equal or left of ctop x1,y1
		clip_vals[2] = clip_vals[2] > ctop_vals[2] ? ctop_vals[2] : clip_vals[2];
		clip_vals[3] = clip_vals[3] > ctop_vals[3] ? ctop_vals[3] : clip_vals[3];

		int new_w = clip_vals[2] - clip_vals[0];
		int new_h = clip_vals[3] - clip_vals[1];

		// Reset clip to new values
		clip.x = clip_vals[0];
		clip.y = clip_vals[1];
		clip.w = new_w < 0 ? 0 : new_w;
		clip.h = new_h < 0 ? 0 : new_h;

		push_clip(clip);
	}
	void pop_clip(void){
		if (render_buffer->_clip_top > 0){
			--render_buffer->_clip_top;
		}
	}
	const render_clip& current_clip(void){
		return render_buffer->_clip_rect[render_buffer->_clip_top];
	}

	void push_glyph(render_glyph glyph){
		float uvs[4];
		get_uvs(glyph.id, uvs);

		// Build vertices
		float xx[2] = {(float)(glyph.x), (float)(glyph.x + glyph.w)};
		float yy[2] = {(float)(glyph.y), (float)(glyph.y + glyph.h)};
		float uu[2] = {uvs[0], uvs[0] + uvs[2]};
		float vv[2] = {uvs[1], uvs[1] + uvs[3]};

		// Clip vertices/uvs

		// If clipped, return early

		// uint8_t fg[4], bg[4];
		// get_color(glyph.fg, fg);
		// get_color(glyph.bg, bg);
		//fg[3] = bg[3] = 255;
		uint8_t fg[4] = {255, 0, 0, 255}, bg[4] = {0, 255, 0, 255};


		// Visible, so go ahead and allocate things
		quad_t quad = {render_buffer->vSize, render_buffer->eSize};
		render_vertex* vert = alloc_vertices(4);
		uint32_t* elem = alloc_elements(6);

		vert = make_vertex(vert, xx[1], yy[1], uu[1], vv[1], bg, fg);
		vert = make_vertex(vert, xx[1], yy[0], uu[1], vv[0], bg, fg);
		vert = make_vertex(vert, xx[0], yy[0], uu[0], vv[0], bg, fg);
		vert = make_vertex(vert, xx[0], yy[1], uu[0], vv[1], bg, fg);

		// Build elements
		elem = make_element(elem, quad.vertex_ndx, 0);
		elem = make_element(elem, quad.vertex_ndx, 1);
		elem = make_element(elem, quad.vertex_ndx, 3);

		elem = make_element(elem, quad.vertex_ndx, 1);
		elem = make_element(elem, quad.vertex_ndx, 2);
		elem = make_element(elem, quad.vertex_ndx, 3);

		// Push to layer
		render_buffer->layers[render_buffer->cLayer].quad.push_back(quad);
	}
	void push_alpha_glyph(render_glyph glyph, uint8_t fg_alpha, uint8_t bg_alpha){
		float uvs[4];
		get_uvs(glyph.id, uvs);

		// Build vertices
		float xx[2] = {(float)(glyph.x), (float)(glyph.x + glyph.w)};
		float yy[2] = {(float)(glyph.y), (float)(glyph.y + glyph.h)};
		float uu[2] = {uvs[0], uvs[0] + uvs[2]};
		float vv[2] = {uvs[1], uvs[1] + uvs[3]};

		// Clip vertices/uvs

		// If clipped, return early

		// uint8_t fg[4], bg[4];
		// get_color(glyph.fg, fg);
		// get_color(glyph.bg, bg);
		uint8_t fg[4] = {255, 0, 0, 255}, bg[4] = {0, 255, 0, 255};
		fg[3] = fg_alpha; bg[3] = bg_alpha;

		// Visible, so go ahead and allocate things
		quad_t quad = {render_buffer->vSize, render_buffer->eSize};
		render_vertex* vert = alloc_vertices(4);
		uint32_t* elem = alloc_elements(6);

		vert = make_vertex(vert, xx[1], yy[1], uu[1], vv[1], bg, fg);
		vert = make_vertex(vert, xx[1], yy[0], uu[1], vv[0], bg, fg);
		vert = make_vertex(vert, xx[0], yy[0], uu[0], vv[0], bg, fg);
		vert = make_vertex(vert, xx[0], yy[1], uu[0], vv[1], bg, fg);

		// Build elements
		elem = make_element(elem, quad.vertex_ndx, 0);
		elem = make_element(elem, quad.vertex_ndx, 1);
		elem = make_element(elem, quad.vertex_ndx, 3);

		elem = make_element(elem, quad.vertex_ndx, 1);
		elem = make_element(elem, quad.vertex_ndx, 2);
		elem = make_element(elem, quad.vertex_ndx, 3);

		// Push to layer
		render_buffer->layers[render_buffer->cLayer].quad.push_back(quad);
	}
	void push_RGBA_glyph(render_glyph glyph, uint8_t* fg, uint8_t* bg){
		// Clip Glyph:
		int clipxy[4] = {0}, glyphxy[4] = {0}, oglyphxy[4] = {0};
		{
			render_clip clip = current_clip();
			clipxy[0] = clip.x;
			clipxy[1] = clip.y;
			clipxy[2] = clip.x + clip.w;
			clipxy[3] = clip.y + clip.h;

			glyphxy[0] = glyph.x;
			glyphxy[1] = glyph.y;
			glyphxy[2] = glyph.x + glyph.w;
			glyphxy[3] = glyph.y + glyph.h;

			oglyphxy[0] = glyphxy[0] > clipxy[0] ? glyphxy[0] : clipxy[0]; // MAX
			oglyphxy[1] = glyphxy[1] > clipxy[1] ? glyphxy[1] : clipxy[1]; // MAX
			oglyphxy[2] = glyphxy[2] < clipxy[2] ? glyphxy[2] : clipxy[2]; // MIN
			oglyphxy[3] = glyphxy[3] < clipxy[3] ? glyphxy[3] : clipxy[3]; // MIN
		}
		if ((oglyphxy[0] < oglyphxy[2]) & (oglyphxy[1] < oglyphxy[3])){
			// Valid clip, can continue!
			float xx[2] = {(float)(oglyphxy[0]), (float)(oglyphxy[2])};
			float yy[2] = {(float)(oglyphxy[1]), (float)(oglyphxy[3])};

			float uvs[4];
			get_uvs(glyph.id, uvs); // u0, v0, du, dv

			float uu[2] = {
				uvs[0] + uvs[2] * ((float)(xx[0] - glyphxy[0]) / (float)glyph.w),
				uvs[0] + uvs[2] * ((float)(xx[1] - glyphxy[0]) / (float)glyph.w)
			};
			float vv[2] = {
				uvs[1] + uvs[3] * ((float)(yy[0] - glyphxy[1]) / (float)glyph.h),
				uvs[1] + uvs[3] * ((float)(yy[1] - glyphxy[1]) / (float)glyph.h)
			};

			// Visible, so go ahead and allocate things
			quad_t quad = {render_buffer->vSize, render_buffer->eSize};
			render_vertex* vert = alloc_vertices(4);
			uint32_t* elem = alloc_elements(6);

			vert = make_vertex(vert, xx[1], yy[1], uu[1], vv[1], bg, fg);
			vert = make_vertex(vert, xx[1], yy[0], uu[1], vv[0], bg, fg);
			vert = make_vertex(vert, xx[0], yy[0], uu[0], vv[0], bg, fg);
			vert = make_vertex(vert, xx[0], yy[1], uu[0], vv[1], bg, fg);

			// Build elements
			elem = make_element(elem, quad.vertex_ndx, 0);
			elem = make_element(elem, quad.vertex_ndx, 1);
			elem = make_element(elem, quad.vertex_ndx, 3);

			elem = make_element(elem, quad.vertex_ndx, 1);
			elem = make_element(elem, quad.vertex_ndx, 2);
			elem = make_element(elem, quad.vertex_ndx, 3);

			// Push to layer
			render_buffer->layers[render_buffer->cLayer].quad.push_back(quad);
		}
	}
	void push_glyph_override(render_glyph glyph, uv_quad uvs, uint8_t* fg, uint8_t* bg){		
		// Build vertices
		float xx[2] = {(float)(glyph.x), (float)(glyph.x + glyph.w)};
		float yy[2] = {(float)(glyph.y), (float)(glyph.y + glyph.h)};
		float uu[2] = {uvs.u, uvs.u + uvs.du};
		float vv[2] = {uvs.v, uvs.v + uvs.dv};

		// Clip vertices/uvs

		// If clipped, return early

		// Visible, so go ahead and allocate things
		quad_t quad = {render_buffer->vSize, render_buffer->eSize};
		render_vertex* vert = alloc_vertices(4);
		uint32_t* elem = alloc_elements(6);

		vert = make_vertex(vert, xx[1], yy[1], uu[1], vv[1], bg, fg);
		vert = make_vertex(vert, xx[1], yy[0], uu[1], vv[0], bg, fg);
		vert = make_vertex(vert, xx[0], yy[0], uu[0], vv[0], bg, fg);
		vert = make_vertex(vert, xx[0], yy[1], uu[0], vv[1], bg, fg);

		// Build elements
		elem = make_element(elem, quad.vertex_ndx, 0);
		elem = make_element(elem, quad.vertex_ndx, 1);
		elem = make_element(elem, quad.vertex_ndx, 3);

		elem = make_element(elem, quad.vertex_ndx, 1);
		elem = make_element(elem, quad.vertex_ndx, 2);
		elem = make_element(elem, quad.vertex_ndx, 3);

		// Push to layer
		render_buffer->layers[render_buffer->cLayer].quad.push_back(quad);
	}
	void push_glyphs(render_glyph* glyph, uint32_t count){
		for (uint32_t i = 0; i < count; ++i){
			push_glyph(glyph[i]);
		}
	}
	void push_alpha_glyphs(render_glyph* glyph, uint32_t count, uint8_t fg_alpha, uint8_t bg_alpha){
		for (uint32_t i = 0; i < count; ++i){
			push_alpha_glyph(glyph[i], fg_alpha, bg_alpha);
		}
	}
	void push_RGBA_glyphs(render_glyph* glyph, uint32_t count, uint8_t* fg, uint8_t* bg){
		for (uint32_t i = 0; i < count; ++i){
			push_RGBA_glyph(glyph[i], fg, bg);
		}
	}

	void push_RGBA_glyphs_ex(render_glyph* glyph, uint32_t count, uv_quad* uv, uint8_t* fg, uint8_t* bg){
		for (uint32_t i = 0; i < count; ++i){
			push_glyph_override(glyph[i], uv[i], fg, bg);
		}
	}
}}