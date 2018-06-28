#pragma once

#include <inttypes.h>

namespace io{ namespace file{
	bool     exists(const char* path);
	uint32_t size  (const char* path);
	bool     write (const char* path, void* data, uint32_t bytes);
	bool     append(const char* path, void* data, uint32_t bytes);
	uint32_t read  (const char* path, void* store, uint32_t bytes);
}}

struct api_file_t{
	bool     (*exists)(const char* path);
	uint32_t (*size)  (const char* path);
	bool     (*write) (const char* path, void* data, uint32_t bytes);
	bool     (*append)(const char* path, void* data, uint32_t bytes);
	uint32_t (*read)  (const char* path, void* store, uint32_t bytes);
};

extern "C" struct api_file_t get_file_api(void);