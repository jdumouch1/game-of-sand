#pragma once
#include <stdint.h>
#include <stdlib.h>
#include <memory.h>

#include "../common/common.h"
#include "cell.h"
#include "sim_data.h"

#define RCHUNK_DREW_EMPTY   0x01
struct render_chunk {
    struct render_cell cells[sizeof(struct render_cell) * CHUNK_AREA];
    size_t cell_count;
    uint8_t flags;
    struct chunk *sim_chunk;
};

#define CHUNK_ACTIVE        0x01
#define CHUNK_FLIP_UPDATE   0x02
#define CHUNK_RENDERED      0x04
struct chunk {
    vec2 pos;
    
    struct cell mesh[CHUNK_AREA];   // Every cell within the chunk
    struct chunk *neighbours[8];    // Pointers to moore neighbour chunks
    uint8_t flags;                  // Flags for the chunk itself
    uvec2 dirt_rect_corners;
    
    struct render_chunk *render_chunk;    
};

static const struct cell EMPTY_CELL = {0};

void chunk_update(struct chunk *c);
void chunk_set_rendered(struct chunk *c, struct render_chunk *rc);
void chunk_set_cell(struct chunk *c, size_t id, struct cell new_cell);

