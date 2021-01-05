#version 460 core
layout (location = 0) in uint cell_data;

uniform uint chunk_size;
uniform uint point_size;

uniform mat4 mvp;
uniform ivec2 chunk_pos;

flat out uint kind;

void main() {
    // id is the first half of the uint. It is the local id in the chunk.
    uint id = cell_data & 0xFFFF;
    
    // TODO: calculate parts of this using matricies
    float x = float((id % chunk_size) + (chunk_pos.x * chunk_size))*point_size;
    float y = float((id / chunk_size) + (chunk_pos.y * chunk_size))*point_size;
    gl_Position = mvp * vec4(x, y, 0.0, 1.0);
    
    // Kind is the other half of the uint, and is sent to determine color
    kind = cell_data >> 16;
}

