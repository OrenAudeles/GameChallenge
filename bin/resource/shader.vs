#version 330 core

layout (location = 0) in vec2 vertex;
layout (location = 1) in vec2 uv;
layout (location = 2) in vec4 col_0;
layout (location = 3) in vec4 col_1;

out vec2 TexCoords;
out vec4 Col_0;
out vec4 Col_1;

uniform mat4 projection;

void main(void){
    TexCoords = uv;
    Col_0 = col_0;
    Col_1 = col_1;

    gl_Position = projection * vec4(vertex, 0.0, 1.0);
}