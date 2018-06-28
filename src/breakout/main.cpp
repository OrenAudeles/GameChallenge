#include <dlfcn.h>
#include <stdio.h>

#include "common/io/file.h"
#include "common/event/event.h"
#include "common/event/event_SDL_enum.h"
#include "common/api.h"

#include <chrono>
#include <thread>

void *challenge_lib = nullptr;
api_common_t challenge_api = {0};

void load_dynamic_libraries(void){
	printf("Attempting to load Challenge Library\n");
	challenge_lib = dlopen("./libchallenge_common.so", RTLD_NOW);

	printf("-- Library Pointer (%p)\n", challenge_lib);

	typedef api_common_t (*api_get)(void);

	api_get get = (api_get)dlsym(challenge_lib, "get_common_api");

	printf("Getting challenge API\n");
	challenge_api = get();

	printf("Checking for 'breakout' existence\n");
	bool breakout_exist = challenge_api.file.exists("breakout");
	printf("-- Breakout Exists? [%c]\n",  breakout_exist ? 'T' : 'F');
	if (breakout_exist){
		printf("-- %u Bytes\n", challenge_api.file.size("breakout"));
	}

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

int main(int argc, const char** argv){
	load_dynamic_libraries();

	challenge_api.graphics.initialize(800, 600, "Test", false);
	challenge_api.event.initialize_handler();

	challenge_api.event.set_event_handler(SDL_QUIT, EVENT_NAME(quit));

	{
		using namespace std::this_thread;
		using namespace std::chrono_literals;
		using std::chrono::high_resolution_clock;

		sleep_until(high_resolution_clock::now() + 1s);
	}

	challenge_api.graphics.set_clear_color(128, 128, 0);

	while (challenge_api.graphics.running()){
		challenge_api.event.poll_events();

		challenge_api.graphics.begin_render();
		
		challenge_api.graphics.end_render();
	}

	challenge_api.event.shutdown_handler();
	challenge_api.graphics.shutdown();

	unload_dynamic_libraries();
	return 0;
}