#pragma once
#include <stdint.h>

enum kind {
    kind_air = 0,
    kind_sand = 1
};

struct kind_property {
    uint32_t color;
    uint8_t density;
};

static const struct kind_property kinds[] = {
    { .color = 0x202020, .density = 0 },   // air
    { .color = 0xf2d2a9, .density = 0 },   // sand
};
