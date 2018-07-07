#version 330 core

in vec2 TexCoords;
in vec4 Col_0;
in vec4 Col_1;

out vec4 color;

uniform sampler2D image;

void main(void){
    vec4 texcol = texture(image, TexCoords);
    
    float gray = (texcol.r + texcol.g + texcol.b) / 3.0;
    vec4 mixcol = mix(Col_0, Col_1, gray);

    if (Col_0 == Col_1){
        mixcol = texcol;
    }
    
    color = vec4(mixcol.rgb, texcol.a);
}