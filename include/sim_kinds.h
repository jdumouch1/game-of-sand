#pragma once
#include <stdint.h>

enum kind {
    kind_air = 0,
    kind_sand,
    kind_water,
};

struct kind_property {
    uint32_t color;
    uint8_t density;
    uint8_t dispersion;
    uint8_t viscosity;
};

static const struct kind_property kinds[] = {
    // 0: Air
    { 
        .color = 0x202020, 
        .density = 16,
        .dispersion = 255,
        .viscosity = 0, 
    },   

    // 1: Sand
    { 
        .color = 0xF2D2A9, 
        .density = 128, 
        .dispersion = 0,
        .viscosity = 255, 
    },  

    // 2: Water
    {   
        .color = 0x0000FF,
        .density = 64,
        .dispersion = 128,
        .viscosity = 64,
    },

    // 3: Gas
    {
        .color = 0xF04040,
        .density = 8,
        .dispersion = 255,
        .viscosity = 0,
    },
};
