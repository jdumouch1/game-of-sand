#pragma once

#include <stdlib.h>
#include <stdint.h>
#include <memory.h>
#include <stdio.h>
#include "common.h"
#include "utils.h"
#include "sim_kinds.h"

#define MAX_VELOCITY 15
#define GRAVITY_ACCEL 2

#define CELL_SETTLED_FLAG  0x01
#define CELL_STATIC_FLAG   0x08

struct cell {
    uint16_t kind;      // 2-bytes
    uint8_t flags;     // 2-bytes
    int8_t vx;        // 1-byte
    int8_t vy;        // 1-byte
    uint8_t r1;
    uint16_t r2;
};

struct universe {
    struct chunk *grid;
};

#define CHUNK_ACTIVE_FLAG   0x01
struct chunk {
    struct cell mesh[CHUNK_AREA];   // Every cell within the chunk
    uint8_t moved[CHUNK_AREA];
    struct chunk *neighbours[8];    // Pointers to moore neighbours
    uint8_t flags;
    vec2 path_buffer[CHUNK_AREA];
};

// Moore neighbor offset for within a chunk
enum neighbor_offset {
    off_ul =  CHUNK_SIZE-1, off_u =  CHUNK_SIZE, off_ur =  CHUNK_SIZE+1,
     
     off_l = -1,                                  off_r =  1,

    off_dl = -CHUNK_SIZE-1, off_d = -CHUNK_SIZE, off_dr = -CHUNK_SIZE+1,
};


static const struct cell EMPTY_CELL = { .kind = 0, .flags = 0, 
                                        .vx = 0, .vy = 0, 
                                        .r1 = 0, .r2 = 0 };

void id_to_uvec2(struct uvec2 *out, size_t local_id);
size_t uvec2_to_id(struct uvec2 *v);

void chunk_update(struct chunk *c, int *order);
void chunk_change_cell(struct chunk *c, size_t id, 
                       uint16_t kind, uint16_t data);

void set_cell(struct chunk *c, size_t id, struct cell new_cell);
