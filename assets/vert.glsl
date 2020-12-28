#version 400 core
layout (location = 0) in vec2 pos;
layout (location = 1) in vec2 tex_coord_in; 

out vec2 tex_coord;

void main(){
    tex_coord = tex_coord_in;
    gl_Position = vec4(pos, 0.0, 1.0);
}

