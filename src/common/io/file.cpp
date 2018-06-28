#include "file.h"

#include <sys/stat.h>
#include <stdio.h>

namespace io{ namespace file{
	bool exists(const char* path){
		struct stat buf;
		return (stat(path, &buf) == 0);
	}
	uint32_t size(const char* path){
		struct stat buf;
		uint32_t sz = 0;

		if (stat(path, &buf) != -1){
			sz = (uint32_t)buf.st_size;
		}
		return sz;
	}
	bool write(const char* path, void* data, uint32_t bytes){
		if (bytes > 0){
			FILE *file = fopen(path, "wb");
			if (!file){
				return false;
			}
			(void)fwrite(data, bytes, 1, file);
			fclose(file);
		}
		return true;
	}
	bool append(const char* path, void* data, uint32_t bytes){
		if (bytes > 0){
			FILE *file = fopen(path, "ab");
			if (!file){
				return false;
			}
			(void)fwrite(data, bytes, 1, file);
			fclose(file);
		}
		return true;
	}
	uint32_t read(const char* path, void* store, uint32_t bytes){
		FILE *file = fopen(path, "rb");
		uint32_t sz = 0;

		if (file){
			sz = size(path);
			sz = sz > bytes ? bytes : sz;
			sz = (uint32_t)fread(store, 1, sz, file);
			fclose(file);
		}

		return sz;
	}
}}
