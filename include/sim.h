#pragma once

#include <stdlib.h>
#include <stdint.h>
#include <memory.h>
#include <stdio.h>
#include "common.h"
#include "utils.h"
#include "sim_kinds.h"
#include "math.h"

#define MAX_SPEED 255
#define GRAVITY_ACCEL 6

#define CELL_SETTLED   0x01
#define CELL_BIAS      0x02
#define CELL_IDLED     0x04
#define CELL_STATIC    0x08
#define CELL_FALLING   0x10
#define CELL_ALTERNATE 0x20
//#define CELL_FLAG         0x40
//#define CELL_FLAG         0x80

// 4-bytes (maximum packing, daddy)
struct cell {
    uint16_t kind;     // 2-bytes
    uint8_t flags;     // 1-byte
    uint8_t speed;     // 1-byte
};

struct universe {
    struct chunk *grid;
};

#define CHUNK_ACTIVE       0x01
#define CHUNK_FLIP_UPDATE  0x02
struct chunk {
    struct cell mesh[CHUNK_AREA];   // Every cell within the chunk
    uint8_t moved[CHUNK_AREA];   
    struct chunk *neighbours[8];    // Pointers to moore neighbour chunks
    uint8_t flags;                  // Flags for the chunk itself
    uvec2 dirt_rect_corners;
};

// Moore neighbor offset for within a chunk
enum neighbor_offset {
    off_ul =  CHUNK_SIZE-1, off_u =  CHUNK_SIZE, off_ur =  CHUNK_SIZE+1,
     
     off_l = -1,                                  off_r =  1,

    off_dl = -CHUNK_SIZE-1, off_d = -CHUNK_SIZE, off_dr = -CHUNK_SIZE+1,
};


static const struct cell EMPTY_CELL = { .kind = 0, .flags = 0, .speed = 0};

void id_to_uvec2(struct uvec2 *out, size_t local_id);
size_t uvec2_to_id(struct uvec2 *v);

void chunk_update(struct chunk *c);
void set_cell(struct chunk *c, size_t id, struct cell new_cell);
