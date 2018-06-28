#include "shader.h"

#include <GL/glew.h>
#include <stdio.h>


static void check_compile_errors(uint32_t shader_id, const char* type_name){
	int success;
	char infoLog[1024];

	glGetShaderiv(shader_id, GL_COMPILE_STATUS, &success);
	if (!success){
		glGetShaderInfoLog(shader_id, 1024, NULL, infoLog);
		printf("ERROR::SHADER_COMPILATION_ERROR of TYPE: %s\n%s\n\n", type_name, infoLog);
	}
}
static void check_link_errors(uint32_t prog_id, const char* type_name){
	int success;
	char infoLog[1024];

	glGetProgramiv(prog_id, GL_LINK_STATUS, &success);
	if (!success){
		glGetProgramInfoLog(prog_id, 1024, NULL, infoLog);
		printf("ERROR::PROGRAM_LINKING_ERROR of TYPE: %s\n%s\n\n", type_name, infoLog);
	}
}

uint32_t shader_create_program(void* vsource, void* fsource, void* gsource, int32_t vlen, int32_t flen, int32_t glen){
	uint32_t ret = 0;

	// It's easiest to check source lengths
	if (vlen && flen){
		uint32_t vid = glCreateShader(GL_VERTEX_SHADER);
		uint32_t fid = glCreateShader(GL_FRAGMENT_SHADER);
		uint32_t gid = 0;

		glShaderSource(vid, 1, (const GLchar* const*)&vsource, &vlen);
		glShaderSource(fid, 1, (const GLchar* const*)&fsource, &flen);

		glCompileShader(vid);
		glCompileShader(fid);

		// Check errors in vid and fid
		check_compile_errors(vid, "VERTEX");
		check_compile_errors(fid, "FRAGMENT");

		if (glen){
			gid = glCreateShader(GL_GEOMETRY_SHADER);
			glShaderSource(gid, 1, (const GLchar* const*)&gsource, &glen);
			glCompileShader(gid);

			// Check errors in gid
			check_compile_errors(gid, "GEOMETRY");
		}

		ret = glCreateProgram();

		glAttachShader(ret, vid);
		glAttachShader(ret, fid);

		if (glen){
			glAttachShader(ret, gid);
		}

		glLinkProgram(ret);

		// Check for link errors
		check_link_errors(ret, "PROGRAM");

		// Detach & Delete Shaders
		glDetachShader(ret, vid);
		glDeleteShader(vid);

		glDetachShader(ret, fid);
		glDeleteShader(fid);

		if (glen){
			glDetachShader(ret, gid);
			glDeleteShader(gid);
		}
	}

	return ret;
}
void shader_destroy_program(uint32_t prog){
	glDeleteProgram(prog);
}

void shader_use_program(uint32_t prog){
	glUseProgram(prog);
}

void shader_set_bool(uint32_t prog, const char* name, const int value){
	glUniform1i(glGetUniformLocation(prog, name), value);
}
void shader_set_int(uint32_t prog, const char* name, const int32_t value){
	glUniform1i(glGetUniformLocation(prog, name), value);
}
void shader_set_float(uint32_t prog, const char* name, const float value){
	glUniform1f(glGetUniformLocation(prog, name), value);
}

void shader_set_vec2(uint32_t prog, const char* name, const float x, const float y){
	glUniform2f(glGetUniformLocation(prog, name), x, y);
}
void shader_set_vec3(uint32_t prog, const char* name, const float x, const float y, const float z){
	glUniform3f(glGetUniformLocation(prog, name), x, y, z);
}
void shader_set_vec4(uint32_t prog, const char* name, const float x, const float y, const float z, const float w){
	glUniform4f(glGetUniformLocation(prog, name), x, y, z, w);
}

void shader_set_vec2v(uint32_t prog, const char* name, const float* value){
	shader_set_vec2(prog, name, value[0], value[1]);
}
void shader_set_vec3v(uint32_t prog, const char* name, const float* value){
	shader_set_vec3(prog, name, value[0], value[1], value[2]);
}
void shader_set_vec4v(uint32_t prog, const char* name, const float* value){
	shader_set_vec4(prog, name, value[0], value[1], value[2], value[3]);
}

void shader_set_mat2(uint32_t prog, const char* name, const float* value){
	glUniformMatrix2fv(glGetUniformLocation(prog, name), 1, GL_FALSE, value);
}
void shader_set_mat3(uint32_t prog, const char* name, const float* value){
	glUniformMatrix3fv(glGetUniformLocation(prog, name), 1, GL_FALSE, value);
}
void shader_set_mat4(uint32_t prog, const char* name, const float* value){
	glUniformMatrix4fv(glGetUniformLocation(prog, name), 1, GL_FALSE, value);
}
