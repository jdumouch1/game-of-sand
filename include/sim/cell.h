#pragma once
#include <stdint.h>
#include <stddef.h>


// Cell flags
#define CELL_SETTLED    0x01
#define CELL_BIAS       0x02
#define CELL_UPDATED    0x04
#define CELL_STATIC     0x08
#define CELL_FALLING    0x10
#define CELL_ALTERNATE  0x20
//#define CELL_RESERVED 0x40
//#define CELL_RESERVED 0x80
struct cell {
    uint16_t kind;
    uint8_t flags;
    uint8_t speed;
};

struct render_cell {
    uint16_t local_id;
    uint16_t kind;
};
