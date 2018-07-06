#include "image.h"
#include "renderer.h"

#include <SDL.h>
#include <SDL_image.h>

#include <stdio.h>

namespace {
	SDL_Surface *open[64] = {0};

	uint32_t get_open_ndx(void){
		for (uint32_t i = 1; i < 64; ++i){
			if (open[i] == nullptr){
				return i;
			}
		}
		return 0;
	}
}

void         image_initialize(void){
	int flags = IMG_INIT_PNG;
	int initted = IMG_Init(flags);
	if ((initted & flags) != flags){
		printf("IMG_Init: Failed to init required PNG support\nIMG_Init: %s\n", IMG_GetError());
	}
}

uint32_t     image_load_lump(void* data, uint32_t data_bytes){
	SDL_RWops *rw = SDL_RWFromMem(data, data_bytes);
    SDL_Surface *temp = IMG_Load_RW(rw, 1);

    // Assumes success
    uint32_t ndx = get_open_ndx();

    SDL_Surface *image = SDL_ConvertSurfaceFormat(temp, SDL_PIXELFORMAT_ABGR8888, 0);
    SDL_FreeSurface(temp);

    open[ndx] = image;

    return ndx;
}
uint32_t     image_load(const char* path){
	SDL_Surface *temp = IMG_Load(path);

	// Assumes success
	uint32_t ndx = get_open_ndx();
	
	//SDL_Surface *image = SDL_ConvertSurfaceFormat(temp, SDL_GetWindowPixelFormat((SDL_Window*)render::get_window()), 0);
	SDL_Surface *image = SDL_ConvertSurfaceFormat(temp, SDL_PIXELFORMAT_ABGR8888, 0);
	SDL_FreeSurface(temp);

	open[ndx] = image;
	return ndx;
}
void         image_free(uint32_t img){
	SDL_FreeSurface(open[img]);
	open[img] = nullptr;
}

image_data_t image_data(uint32_t img){
	const SDL_Surface* surface = open[img];

	image_data_t result = {0};
	result.width = surface->w;
	result.height = surface->h;
	result.bpp = surface->format->BytesPerPixel;
	result.data = surface->pixels;

	return result;
}