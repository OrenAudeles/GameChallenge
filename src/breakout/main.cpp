#include <dlfcn.h>
#include <stdio.h>

#include "common/io/file.h"

void *challenge_lib = nullptr;

void load_dynamic_libraries(void){
	printf("Attempting to load Challenge Library\n");
	challenge_lib = dlopen("./libchallenge_common.so", RTLD_NOW);

	printf("-- Library Pointer (%p)\n", challenge_lib);

	typedef api_file_t (*api_file_get)(void);

	api_file_get get = (api_file_get)dlsym(challenge_lib, "get_file_api");

	api_file_t api = get();

	bool breakout_exist = api.exists("breakout");
	printf("Breakout Exists? [%c]\n",  breakout_exist ? 'T' : 'F');
	if (breakout_exist){
		printf("-- %u Bytes\n", api.size("breakout"));
	}
}
void unload_dynamic_libraries(void){
	dlclose(challenge_lib);
	challenge_lib = nullptr;
}

int main(int argc, const char** argv){
	load_dynamic_libraries();

	unload_dynamic_libraries();
	return 0;
}