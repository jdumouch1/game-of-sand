#pragma once
#include <stdlib.h>
#include <stdint.h>
#include <memory.h>
#include <stdio.h>
#include "common.h"
#include "utils.h"
#include "sim_kinds.h"

#define CELL_STATE_SETTLED  0x01
#define CELL_DATA_VELOCITY  0x0F
struct cell {
    uint16_t kind;
    uint8_t states;
    uint8_t data;
};

struct universe {
    struct chunk *grid;
};

#define CHUNK_FLAG_ACTIVE   0x01
struct chunk {
    struct cell mesh[CHUNK_AREA];   // Every cell within the chunk
    uint8_t updated[CHUNK_AREA];
    struct chunk *neighbours[8];    // Pointers to moore neighbours
    uint8_t flags;
};

// Moore neighbor offset for within a chunk
enum neighbor_offset {
    nbr_ul =  CHUNK_SIZE-1, nbr_u =  CHUNK_SIZE, nbr_ur =  CHUNK_SIZE+1,
     
     nbr_l = -1,                                  nbr_r =  1,

    nbr_dl = -CHUNK_SIZE-1, nbr_d = -CHUNK_SIZE, nbr_dr = -CHUNK_SIZE+1,
};

static const struct cell EMPTY_CELL = { .kind = 0, .states = 0, .data = 0 };

void id_to_uvec2(struct uvec2 *out, size_t local_id);
size_t uvec2_to_id(struct uvec2 *v);

void chunk_update(struct chunk *c);
