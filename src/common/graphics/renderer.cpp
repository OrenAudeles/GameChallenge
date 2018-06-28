#include "renderer.h"
#include "image.h"

#include <SDL.h>
#include <GL/glew.h>

#include <new>

struct SDL_Renderer{
	SDL_Window* window;
	SDL_GLContext glcontext;
	bool running;

	SDL_Renderer(void):window(0), glcontext(0), running(false){}
};

namespace {
	static uint8_t buffer_renderer[sizeof(SDL_Renderer)];
	SDL_Renderer* renderer = 0;
}

namespace render{
	void initialize(int window_width, int window_height, const char* window_title, bool resizable){
		image_initialize();

		if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) != 0){
			printf("SDL failed to initialize!\n");
			return;
		}

		renderer = new (buffer_renderer) SDL_Renderer();
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG);
	    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	    //SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
	    //SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
	    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE); //OpenGL core profile
	    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3); //OpenGL 3+
	    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3); //OpenGL 3.3

	    SDL_DisplayMode current;
	    SDL_GetCurrentDisplayMode(0, &current);

	    uint32_t flags = SDL_WINDOW_OPENGL| (resizable * SDL_WINDOW_RESIZABLE);

	    /** SDL_Window **/renderer->window = SDL_CreateWindow(window_title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, window_width, window_height, flags);

	    /** SDL_GLContext **/ renderer->glcontext = SDL_GL_CreateContext(renderer->window);
	    // Initialize GL
	    glewExperimental = GL_TRUE;
	    if (glewInit() != GLEW_OK){
	    	printf("GLEW failed to initialize!\n");
	    	shutdown();
	    	return;
	    }
	    glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		renderer->running = true;
		printf("SDL and GLEW initialized\n");
	}
	void shutdown(void){
		SDL_GL_DeleteContext(renderer->glcontext);
		SDL_DestroyWindow(renderer->window);
		SDL_Quit();

		renderer = nullptr;
	}
	bool running(void){
		return (renderer != nullptr) && (renderer->running == true);
		//return renderer->running;
	}
	void close_window(void){
		renderer->running = false;
	}

	RenderWindow* get_window(void){
		return (RenderWindow*)renderer->window;
	}

	void begin_render(void){
		clear();
		int width, height;
		drawable_size(width, height);
		viewport(0, 0, width, height);
	}
	void end_render(void){
		SDL_GL_SwapWindow(renderer->window);
	}
	void clear(void){
		glClear(GL_COLOR_BUFFER_BIT);
	}
	void set_clear_color(uint8_t r, uint8_t g, uint8_t b){
		const float mul = 1 / 255.f;
		glClearColor(r * mul, g * mul, b * mul, 1);
	}

	void set_window_title(const char* title){
		SDL_SetWindowTitle(renderer->window, title);
	}

	void viewport(int left_x, int top_y, int width, int height){
		glViewport(left_x, top_y, width, height);
	}

	void window_size(int& width, int& height){
		SDL_GetWindowSize(renderer->window, &width, &height);
	}
	void drawable_size(int& width, int& height){
		SDL_GL_GetDrawableSize(renderer->window, &width, &height);
	}
}