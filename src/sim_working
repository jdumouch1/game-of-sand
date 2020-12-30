#include "../include/sim.h"
#include <bits/stdint-uintn.h>


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

int chunk_raycast(struct chunk *c, int *id, 
                      uint8_t *distance, int delta){
    int start = *id;
    uint8_t max_distance = *distance;
    for (int i = 0; i < max_distance; i++){
        if ((*id+delta) < 0 ||  
            c->mesh[*id+delta].kind){ break; }
        
        *distance -= 1;
        *id += delta;
    }
    
    return *id != start;
}

void cell_update(struct chunk *c, size_t local_id){
    // Get cell pointer
    struct cell *cell = &c->mesh[local_id];
    
    // Calculate local coords of cell.
    struct uvec2 pos;
    id_to_uvec2(&pos, local_id);
    if (!pos.y) { return; }
        
    int primary[1] = { nbr_d };
    int secondary[2] = {nbr_dl, nbr_dr};

    uint8_t vel = (cell->data & CELL_DATA_VELOCITY) + 1;
    int new_id = local_id;
    while (vel){
        if (!chunk_raycast(c, &new_id, &vel, nbr_d)){
            shuffle(secondary, 2);
            if (!chunk_raycast(c, &new_id, &vel, secondary[0]) &&
                !chunk_raycast(c, &new_id, &vel, secondary[1])){
                vel = 0; 
            }else{
                vel>>=1;
            }
        }
    }

    if (new_id != local_id){
        struct cell *new_cell = &c->mesh[new_id];
        memcpy(new_cell, cell, sizeof(struct cell));
        c->updated[new_id] |= 0x1;
        if ((new_cell->data & CELL_DATA_VELOCITY) < 15){
            new_cell->data++;
        }
        *cell = EMPTY_CELL;
        
        // Unsettle neighbors
        c->mesh[local_id + nbr_u].states &= ~(CELL_STATE_SETTLED);
        //c->mesh[local_id + nbr_ul].states &= ~(CELL_STATE_SETTLED);
        //c->mesh[local_id + nbr_ur].states &= ~(CELL_STATE_SETTLED);
    }else{
        cell->data &= ~(CELL_DATA_VELOCITY);    // Zero the velocity
        cell->states |= CELL_STATE_SETTLED;     // Set to settled state
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
    // Generate an odd number in (0, CHUNK_SIZE)
    // Using the definition of odd numbers a = (2b)+1
    // b = CHUNK_SIZE/2 - 1 (because of the +1 at the end)
    size_t coprime = (rand() % (CHUNK_SIZE/2-1)) * 2 + 1;
    
    // Set the chunk to inactive, in case no cells are updated.
    c->flags &= ~(CHUNK_FLAG_ACTIVE);   

    // Loop through each cell in random order
    size_t i = 0;
    while ( i < CHUNK_AREA ) {
        // Generate a unique index using (a*x) % n
        // which given i reaches all values up to n, will 
        // generate every number up to n, with a pseudo-random offset
        size_t next = (i * coprime) % CHUNK_AREA;
        struct cell *cell = &c->mesh[next];

        // Check if the cell exists and has not settled
        if (cell->kind && 
            !(cell->states & CELL_STATE_SETTLED) && 
            !c->updated[next]){
            
            cell_update(c, next);

            c->flags |= 0x1; // Given that a cell has moved, the chunk
                             // must be active.
        }

        i++;
    }
}
