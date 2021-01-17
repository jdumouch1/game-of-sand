#include "../../include/sim/universe.h"

void universe_spawn_chunk(struct universe *u, vec2 pos){
    // Create the chunk
    u->grid[u->num_chunks++] = (struct chunk) {
        .pos = pos,
    };
    struct chunk *new_chunk = &u->grid[u->num_chunks-1];
    // Neighbours are arranged as:
    // 0 1 2
    // 3   4
    // 5 6 7
    
    // Link neighbours
    for (size_t i = 0; i < u->num_chunks; i++){
        struct chunk *c = &u->grid[i];
        if (pos.x == c->pos.x){
            // Up
            if (pos.y+1 == c->pos.y){
                new_chunk->neighbours[1] = c; 
                u->grid[i].neighbours[6] = new_chunk;
            }
            // Down
            else if (pos.y-1 == c->pos.y){
                new_chunk->neighbours[6] = c;
                u->grid[i].neighbours[1] = new_chunk;
            }
        }
        else if (pos.x-1 == c->pos.x){
            // Up-Left
            if (pos.y+1 == c->pos.y){
                new_chunk->neighbours[0] = c;
                u->grid[i].neighbours[7] = new_chunk;
            }
            // Left 
            else if (pos.y == c->pos.y){
                new_chunk->neighbours[3] = c;
                u->grid[i].neighbours[4] = new_chunk;
            }
            // Down-Left
            else if (pos.y-1 == c->pos.y){
                new_chunk->neighbours[5] = c;
                u->grid[i].neighbours[2] = new_chunk;
            }
        }
        else if (pos.x+1 == c->pos.x){
            // Up-Right
            if (pos.y+1 == c->pos.y){
                new_chunk->neighbours[2] = c;
                u->grid[i].neighbours[5] = new_chunk;
            }
            // Right
            else if (pos.y == c->pos.y){
                new_chunk->neighbours[4] = c;
                u->grid[i].neighbours[3] = new_chunk;
            }
            // Down-Right
            else if (pos.y == c->pos.y + 1){
                new_chunk->neighbours[7] = c;
                u->grid[i].neighbours[0] = new_chunk;
            }
        }
    }
}

void universe_set_cell(struct universe *u, vec2 world_pos, 
                       struct cell cell_data){

    vec2 local_pos = world_to_local(world_pos);
    vec2 chunk_pos = (vec2) {
        .x = world_pos.x / CHUNK_SIZE,
        .y = world_pos.y / CHUNK_SIZE
    };
    struct chunk *chunk = NULL;
    for (size_t i = 0; i < u->num_chunks; i++){
        struct chunk *c = &u->grid[i];
        if (chunk_pos.x == c->pos.x && chunk_pos.y == c->pos.y){
            chunk = c;
            break; 
        }
    }
    if (!chunk) { goto out; }

    size_t cell_id = local_pos.x + local_pos.y*CHUNK_SIZE;
    chunk->mesh[cell_id] = cell_data;

out:
    return;
}
