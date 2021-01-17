#pragma once 
#include <stddef.h>
#include "../common/common.h"
#include "cell.h"
#include "chunk.h"
#include "sim_data.h"

inline vec2 local_to_world(struct chunk *c, vec2 local){
    return (vec2) {
        .x = (int16_t)(c->pos.x*CHUNK_SIZE + local.x),
        .y = (int16_t)(c->pos.y*CHUNK_SIZE + local.y),
    };
}
inline vec2 world_to_local(vec2 world){
    return (vec2) {
        .x = (int16_t)(world.x % CHUNK_SIZE),
        .y = (int16_t)(world.y % CHUNK_SIZE)
    };
}

struct universe {
    struct chunk *grid;
    size_t num_chunks;
};

void universe_spawn_chunk(struct universe *u, vec2 pos);
void universe_set_cell(struct universe *u, vec2 world_pos, 
                       struct cell cell_data);




