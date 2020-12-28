#include "../include/sim.h"


inline void id_to_uvec2(struct uvec2 *out, size_t local_id){
    out->x = local_id % CHUNK_SIZE;
    out->y = local_id >> CHUNK_SCALE;

    // Note: CHUNK_SIZE is a power of two and division is optimized by
    // using a left shift by Log2(CHUNK_SIZE), which happens to be CHUNK_SCALE
}

inline size_t uvec2_to_id(struct uvec2 *v){
    return v->x + (v->y << CHUNK_SCALE);
    
    // Note: Id is calculated as the x + y * row_size, 
    // y * row_size is optimized by using y << Log2(CHUNK_SIZE), known to be
    // y << CHUNK_SCALE
}


void cell_update(struct chunk *c, size_t local_id){
    // Get cell pointer
    struct cell *cell = &c->mesh[local_id];
    
    // Calculate local coords of cell.
    struct uvec2 pos;
    id_to_uvec2(&pos, local_id);
    if (!pos.y) { return; }

    // Calculate local id of valid moore neighbors
    size_t neighbors[8] = {
        local_id-CHUNK_SIZE,
        local_id-CHUNK_SIZE - 1,
        local_id-CHUNK_SIZE + 1,
        local_id+CHUNK_SIZE,
        local_id+CHUNK_SIZE - 1,
        local_id+CHUNK_SIZE + 1,
        local_id-1,
        local_id+1,
    };
    int valid_nbrs = 8;
    uint8_t vel = (cell->data & CELL_DATA_VELOCITY) + 1;
    int move_id = local_id;
    for (int i = 0; i < vel; i++){
        if (c->mesh[move_id-CHUNK_SIZE].kind || move_id < CHUNK_SIZE){ break; } 
        move_id -= CHUNK_SIZE;
    }
    
    if (move_id != local_id){
        struct cell *new_cell = &c->mesh[move_id];
        memcpy(new_cell, cell, sizeof(struct cell));
        if (vel < 15){ new_cell->data++; }
        c->updated[move_id] |= 0x1;
        
        *cell = EMPTY_CELL;

        c->mesh[local_id + CHUNK_SIZE].states &= ~(CELL_STATE_SETTLED);
        c->mesh[local_id + CHUNK_SIZE-1].states &= ~(CELL_STATE_SETTLED);
        c->mesh[local_id + CHUNK_SIZE+1].states &= ~(CELL_STATE_SETTLED);
    }else{
        cell->states |= CELL_STATE_SETTLED;
    }

    /*if (!c->mesh[move_id].kind && !c->updated[move_id]){
        struct cell *nbr = &c->mesh[move_id];
        // Space is empty, move to it, leaving empty behind
        memcpy(nbr, cell, sizeof(struct cell));
        c->updated[move_id] |= 0x1;
        c->updated[local_id] = 0;//x1;

        *cell = EMPTY_CELL;
        for (int j = 0; j<valid_nbrs; j++){
            c->mesh[neighbors[j]].states &= (~CELL_STATE_SETTLED);
        }
        return;
    }

    cell->states |= CELL_STATE_SETTLED;
    */
}

void chunk_update(struct chunk *c){
    // Reset the cell updated states
    memset(&c->updated, 0, sizeof(c->updated));

    for (size_t i = 0; i < CHUNK_AREA; i++){
        struct cell *cell = &c->mesh[i];
        // Check if the cell exists and is moving
        if (cell->kind && 
            !(cell->states & CELL_STATE_SETTLED) && 
            !c->updated[i]){
            
            cell_update(c, i);
        }
    }
}
