#include "../include/sim.h"
#include <bits/stdint-uintn.h>

void cell_update(struct chunk *c, size_t id);

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

void chunk_update(struct chunk *c){
    // Reset the cell updated states
    memset(&c->updated, 0, sizeof(c->updated));

    // Generate an odd number in (0, CHUNK_SIZE)
    // Using the definition of odd numbers a = (2b)+1
    // b = CHUNK_SIZE/2 - 1 (because of the +1 at the end)
    size_t coprime = (rand() % (CHUNK_SIZE/2-1)) * 2 + 1;
    
    // Set the chunk to inactive, in case no cells are updated.
    CLR_FLAG(c->flags, CHUNK_ACTIVE_FLAG);
    // Loop through each cell in random order
    size_t i = 0;
    while ( i < CHUNK_AREA ) {
        // Generate a unique index using (a*x) % n
        // Provided i reaches all values up to n, will 
        // generate every number up to n, with a pseudo-random offset
        size_t next = (i * coprime) % (CHUNK_AREA);
        struct cell *cell = &c->mesh[next];

        // Check if the cell exists and has not settled
        if (cell->kind){

            cell_update(c, next);
            
            // Set that has been an update in this chunk
            SET_FLAG(c->flags, CHUNK_ACTIVE_FLAG);
        }

        i++;
    }
}

int cell_can_move(struct chunk *c, size_t id, int delta){
    int can_move = 1;
    if (id < CHUNK_SIZE) { 
        can_move = 0;
        goto out;
    }
    
    size_t to = id + delta;
    if (c->updated[to] || c->mesh[to].kind){
        can_move = 0;
        goto out;
    }

out:
    return can_move;
}

void cell_update(struct chunk *c, size_t id){
    struct cell *cell = &c->mesh[id];
    int velocity = CELL_VELOCITY(cell->data);
    int travel = velocity+1;

    while (travel){
        if (id < CHUNK_SIZE) { goto no_move; }
        int new_id;
        
        // First try moving down
        if (cell_can_move(c, id, off_d)){
            new_id = id+off_d;
        }
        // Otherwise, try moving down and to the sides
        else {
            int choice = rand()%2;
            int deltas[2] = { off_dl, off_dr };

            if (cell_can_move(c, id, deltas[choice])){
                new_id = id+deltas[choice];
            }
            else if (cell_can_move(c, id, deltas[(choice+1)%2])){
                new_id = id+deltas[(choice+1)%2];
            }else{
                goto no_move;
            }
        }
        
        c->mesh[new_id] = *cell;
        *cell = EMPTY_CELL;

        travel--;
    }
        

    return;

no_move:
    // Stop, settle and mark the cell as updated
    CELL_VELOCITY_CLR(c->mesh[id].data);
    SET_FLAG(c->mesh[id].data, CELL_SETTLED_FLAG);
    c->updated[id] = 1;
}

