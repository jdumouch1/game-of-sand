#pragma once
#include <stdlib.h>
#include <stdint.h>
#include <memory.h>
#include <stdio.h>
#include "common.h"
#include "utils.h"
#include "sim_kinds.h"

#define MAX_VELOCITY 15
#define GRAVITY_ACCEL 1
#define CELL_VELOCITY(n)        ((((n) >> 8) & 0xFF))
#define CELL_VELOCITY_CLR(n)    (((n) &= ~(0xFF<<8)))
#define CELL_VELOCITY_SET(n, v) (CELL_VELOCITY_CLR((n))); (((n) |= ((v)<<8)))

#define CELL_VARIANT(n)         ((((n) >> 1) & 0x3))

#define CELL_SETTLED_FLAG  0x01
// Data packing:
// 0        : Settled Flag
// 1 to 2   : Variant (range: 0 to 3)
// 3-6      : Reserved
// 7-15     : Velocity (range: 0 to 15)
struct cell {
    uint16_t kind;
    uint16_t data;
};

struct universe {
    struct chunk *grid;
};

#define CHUNK_ACTIVE_FLAG   0x01
struct chunk {
    struct cell mesh[CHUNK_AREA];   // Every cell within the chunk
    uint8_t updated[CHUNK_AREA];
    struct chunk *neighbours[8];    // Pointers to moore neighbours
    uint8_t flags;
};

// Moore neighbor offset for within a chunk
enum neighbor_offset {
    off_ul =  CHUNK_SIZE-1, off_u =  CHUNK_SIZE, off_ur =  CHUNK_SIZE+1,
     
     off_l = -1,                                  off_r =  1,

    off_dl = -CHUNK_SIZE-1, off_d = -CHUNK_SIZE, off_dr = -CHUNK_SIZE+1,
};


static const struct cell EMPTY_CELL = { .kind = 0, .data = 0};

void id_to_uvec2(struct uvec2 *out, size_t local_id);
size_t uvec2_to_id(struct uvec2 *v);

void chunk_update(struct chunk *c);
void chunk_change_cell(struct chunk *c, size_t id, 
                       uint16_t kind, uint16_t data);
