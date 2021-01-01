#pragma once
#include <stdint.h>

enum kind {
    kind_air        = 0,
    kind_sand       = 1,
    kind_water      = 2,
    kind_gas        = 3,
    kind_stone      = 4,
};

struct kind_property {
    char name[8];
    uint32_t color;
    uint8_t density;
    uint8_t dispersion;
    uint8_t friction;
};

static const struct kind_property kinds[] = {
    // 0: Air
    { 
        .name = "Air",
        .color = 0x202020, 
        .density = 16,
        .dispersion = 255,
        .friction = 0, 
    },   

    // 1: Sand
    { 
        .name = "Sand",
        .color = 0xF2D2A9, 
        .density = 128, 
        .dispersion = 0,
        .friction = 64, 
    },  

    // 2: Water
    {   
        .name = "Water",
        .color = 0x0000FF,
        .density = 64,
        .dispersion = 200,
        .friction = 0,
    },

    // 3: Gas
    {
        .name = "Gas",
        .color = 0xC0FFC0,
        .density = 8,
        .dispersion = 255,
        .friction = 0,
    },

    // 4: Stone
    {
        .name = "Stone",
        .color = 0xD0D0D0,
        .density = 255,
        .dispersion = 0,
        .friction = 240,       
    }
};
