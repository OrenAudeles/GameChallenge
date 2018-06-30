#include "file.h"


#include <sys/stat.h>
#include <dirent.h>
#include <string.h>

static bool str_ends_with(const char* str, const uint32_t str_len, const char* end, const uint32_t end_len){
	bool result = false;

	if (str_len >= end_len){
		result = (0 == memcmp(end, str + (str_len - end_len), end_len));
	}

	return result;
}

static uint32_t str_copy(void* dest, const char* str, const uint32_t str_len){
	memcpy(dest, str, str_len);
	return str_len;
}
static uint32_t str_copy_if(int condition, void* dest, const char* str, const uint32_t str_len){
	return condition ? str_copy(dest, str, str_len) : 0;
}

static DIR* open_directory(const char* path){
	DIR *dr = opendir(path);
	return dr;
}
static DIR* open_directory_if(int condition, const char* path){
	return condition ? open_directory(path) : 0;
}

namespace io{ namespace file{
	uint32_t find(const char* root_path, const char* ext, void* store, uint32_t store_bytes, uint32_t& store_bytes_used){
		store_bytes_used = 0;

		struct dirent *de = 0;

		DIR *dr = open_directory(root_path);

		uint32_t result = 0;

		uint8_t* store_head = (uint8_t*)store;
		uint32_t root_path_len = strlen(root_path);
		uint32_t file_path_len = 0;
		const uint32_t ext_len = strlen(ext);

		if (dr != 0){
			while((de = readdir(dr))){
				int is_dir = 0;
				#ifdef _DIRENT_HAVE_D_TYPE
				if (de->d_type != DT_UNKNOWN && de->d_type != DT_LNK){
					is_dir = (de->d_type == DT_DIR);
				}else
				#endif
				{
					struct stat buf;
					stat(de->d_name, &buf);
					is_dir = S_ISDIR(buf.st_mode);
				}

				if (!is_dir){
					file_path_len = strlen(de->d_name);
					// Test extension
					if (str_ends_with(de->d_name, file_path_len, ext, ext_len)){
						++result;
						// Copy into head
						// Can copy entire path
						int can_copy = store_bytes > (store_bytes_used + 2 + root_path_len + file_path_len);
						// Copy root path in
						store_bytes_used += str_copy_if(can_copy, store_head + store_bytes_used, root_path, root_path_len);
						// Copy in separator
						store_bytes_used += str_copy_if(can_copy, store_head + store_bytes_used, "/", 1);
						// Copy in file path
						store_bytes_used += str_copy_if(can_copy, store_head + store_bytes_used, de->d_name, file_path_len);
						// Copy in null-terminator
						store_bytes_used += str_copy_if(can_copy, store_head + store_bytes_used, "\0", 1);
					}
				}
			}
		}
		closedir(dr);

		return result;
	}
	uint32_t find_recursive(const char* root_path, const char* ext, void* store, uint32_t store_bytes, uint32_t& store_bytes_used){
		struct dirent* de = 0;

		#define MAX_STACK_DEPTH (32)
		int stack_end_of_dir_path[MAX_STACK_DEPTH];
		DIR *open_dir[MAX_STACK_DEPTH] = {0};

		int in_stack = 0;

		char full_path[1024] = {0};
		uint32_t root_path_len = strlen(root_path);
		uint32_t file_path_len = 0;

		memcpy(full_path, root_path, root_path_len);
		stack_end_of_dir_path[in_stack++] = root_path_len;

		uint32_t result = 0;
		const uint32_t ext_len = strlen(ext);

		uint8_t* store_head = (uint8_t*)store;

		if ((open_dir[in_stack-1] = open_directory(full_path)) != 0){
			while (in_stack > 0){
				while ((de = readdir(open_dir[in_stack-1]))){
					int cur_eos = stack_end_of_dir_path[in_stack-1];
					// Ensure that the path ends with "\/0"
					(void)str_copy(full_path + cur_eos, "/\0", 2);

					// Mostly the same as file_find for a bit
					int is_dir = 0;
					#ifdef _DIRENT_HAVE_D_TYPE
					if (de->d_type != DT_UNKNOWN && de->d_type != DT_LNK){
						is_dir = (de->d_type == DT_DIR);
					}else
					#endif
					{
						struct stat buf;
						stat(de->d_name, &buf);
						is_dir = S_ISDIR(buf.st_mode);
					}

					file_path_len = strlen(de->d_name);
					if (!is_dir){
						// Test extension
						if (str_ends_with(de->d_name, file_path_len, ext, ext_len)){
							++result;
							// Copy into head
							// Can copy entire path
							int can_copy = store_bytes > (store_bytes_used + 1 + cur_eos + file_path_len);
							// Copy current full path
							store_bytes_used += str_copy_if(can_copy, store_head + store_bytes_used, full_path, cur_eos + 1); // <path> + '/'
							// Copy in file path
							store_bytes_used += str_copy_if(can_copy, store_head + store_bytes_used, de->d_name, file_path_len);
							// Copy in null-terminator
							store_bytes_used += str_copy_if(can_copy, store_head + store_bytes_used, "\0", 1);
						}
					}
					else{
						// is a directory!
						// Ignore directories beginning in '.'
						int can_copy = (de->d_name[0] != '.');
						// Push the name onto the full path
						// Copy directory path after [cur_path]/
						int path_added = cur_eos + 1;
						path_added += str_copy_if(can_copy, full_path + path_added, de->d_name, file_path_len);
						// Add null-terminator
						path_added += str_copy_if(can_copy, full_path + path_added, "\0", 1);
						stack_end_of_dir_path[in_stack] = path_added - 1; // Path len is sans nullterminator
						// Try to open the directory
						open_dir[in_stack] = open_directory_if(can_copy, full_path);
						in_stack += (open_dir[in_stack] != 0);
					}
				}
				// Close the open dir
				closedir(open_dir[in_stack-1]);
				open_dir[in_stack-1] = 0;
				// Pop stack
				--in_stack;
			}
		}
		if (open_dir[in_stack]){
			closedir(open_dir[in_stack]);
		}

		return result;
	}
}}