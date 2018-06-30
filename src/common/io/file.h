#pragma once

#include <inttypes.h>

namespace io{ namespace file{
	bool     exists(const char* path);
	uint32_t size  (const char* path);
	bool     write (const char* path, void* data, uint32_t bytes);
	bool     append(const char* path, void* data, uint32_t bytes);
	uint32_t read  (const char* path, void* store, uint32_t bytes);

	uint32_t find(const char* root, const char* ext, void* store, uint32_t store_bytes, uint32_t& store_bytes_used);
	uint32_t find_recursive(const char* root, const char* ext, void* store, uint32_t store_bytes, uint32_t& store_bytes_used);
}}
