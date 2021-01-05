#version 460 core
layout (std430, binding = 1) buffer kind_data {
    uint colors[];
};

flat in uint kind;

out vec4 FragColor;

void main() {
    uint color_int = colors[kind];
    float r = float((color_int >> 16) & 0xFF) / 255.0;
    float g = float((color_int >> 8 ) & 0xFF) / 255.0;
    float b = float((color_int >> 0 ) & 0xFF) / 255.0;
    FragColor = vec4(r, g, b, 1.0);
}


