#include "free.h"

#include <SDL.h>

void memory_free(void* mem){
	SDL_free(mem);
}