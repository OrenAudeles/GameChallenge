#include "event.h"

#include <SDL.h>

void event::poll_events(void){
	SDL_Event ev;
	while (SDL_PollEvent(&ev)){
		call_event_handler(ev.type, &ev);
		//quit(ev);
	}
}
void event::wait_events(void){
	SDL_Event ev;
	if (SDL_WaitEvent(&ev)){
		call_event_handler(ev.type, &ev);
		//quit(ev);
	}
}
void event::wait_events_timeout(float timeout){
	SDL_Event ev;
	while (SDL_WaitEventTimeout(&ev, timeout)){
		call_event_handler(ev.type, &ev);
		//quit(ev);
	}
}