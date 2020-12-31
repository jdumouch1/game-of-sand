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

void chunk_update(struct chunk *c, int *order){
    // Reset the cell updated states
    memset(&c->moved, 0, sizeof(c->moved));
    // Set the chunk to inactive, in case no cells are updated.
    CLR_FLAG(c->flags, CHUNK_ACTIVE_FLAG);
    for (size_t i = 0; i < CHUNK_AREA; i++){
        int selected_id = order[i];
        // generate every number up to n, with a pseudo-random offset
        struct cell *cell = &c->mesh[selected_id];
        // Check if the cell exists and has not settled
        if (!c->moved[selected_id]
            && !CHK_FLAG(cell->data, CELL_SETTLED_FLAG)
            && !CHK_FLAG(cell->data, CELL_STATIC_FLAG)){
            struct uvec2 p;
            id_to_uvec2(&p, selected_id);
            cell_update(c, selected_id);
            
            // Set that has been an update in this chunk
            SET_FLAG(c->flags, CHUNK_ACTIVE_FLAG);
        }
    }
}

int cell_can_move(struct chunk *c, size_t id, int delta){
    int can_move = 1;
    // Bounds check
    if ((int)(id)+delta < 0 || id+delta > CHUNK_AREA){ 
        can_move = 0;
        goto out;
    }
    size_t to = id + delta;
    if (c->moved[to]){
        can_move = 0;
        goto out;
    }
    uint8_t from_density = kinds[c->mesh[id].kind].density; 
    uint8_t to_density = kinds[c->mesh[to].kind].density;
    if (to_density >= from_density){
        can_move = 0;
        goto out;
    }
out:
    return can_move;
}

// TODO: Optimize
int in_x_bounds(size_t id, int delta){
    int is_left = delta/CHUNK_SIZE != 0;
    if ((is_left && (id%CHUNK_SIZE) == 0) 
        || (!is_left && (id%CHUNK_SIZE)==CHUNK_SIZE-1)){
        return 0;
    
    }

    return 1;
}

size_t cell_move(struct chunk *c, size_t from, size_t to){
    struct cell *cell = &c->mesh[from];
    struct cell tmp = c->mesh[from];
    struct cell *new_cell = &c->mesh[to];
    if (cell->kind == new_cell->kind){}
    memcpy(cell, new_cell, sizeof(struct cell));
    memcpy(new_cell, &tmp, sizeof(struct cell));
    cell = new_cell;

    if (from < CHUNK_AREA-CHUNK_SIZE){
        CLR_FLAG(c->mesh[from + off_u].data, CELL_SETTLED_FLAG); 
        CLR_FLAG(c->mesh[from + off_ul].data, CELL_SETTLED_FLAG); 
        CLR_FLAG(c->mesh[from + off_ur].data, CELL_SETTLED_FLAG);
    }
    if (from < CHUNK_AREA){
        CLR_FLAG(c->mesh[from + off_r].data, CELL_SETTLED_FLAG);
    }
    if (from > -1){
        CLR_FLAG(c->mesh[from + off_l].data, CELL_SETTLED_FLAG);
    }
    if (from > CHUNK_SIZE){
        CLR_FLAG(c->mesh[from + off_d].data, CELL_SETTLED_FLAG);
        CLR_FLAG(c->mesh[from + off_dr].data, CELL_SETTLED_FLAG);
        CLR_FLAG(c->mesh[from + off_dl].data, CELL_SETTLED_FLAG);
    }
    return to;
}

void cell_update(struct chunk *c, size_t id){
    struct cell *cell = &c->mesh[id];
    int velocity = CELL_VELOCITY(cell->data);
    int travel = velocity+1;

travel_start:
    while (travel){
        if (id < CHUNK_SIZE) { goto settled; }
        
        // First try moving down
        if (cell_can_move(c, id, off_d)){
            
            id = cell_move(c, id, id + off_d);
            travel--;
            continue;
        }
        
        // Try down and to the sides
        int deltas[2] = { off_dl, off_dr };
        shuffle(deltas, 2);
        for (int i = 0; i < 2; i++){
            if (cell_can_move(c, id, deltas[i]) 
                && in_x_bounds(id, deltas[i])){
                
                id = cell_move(c, id, id+deltas[i]);
                travel--;
                goto travel_start;
            }
        }
        // Try to disperse (left and right)
        uint8_t dispersion = kinds[c->mesh[id].kind].dispersion;
        if (dispersion){
            deltas[0] = off_r; 
            deltas[1] = off_l;
            shuffle(deltas, 2);
            int moved = 0;
            for (int i = 0; i < 2; i++){
                if (cell_can_move(c, id, deltas[i])
                    && in_x_bounds(id, deltas[i])){
                
                    id = cell_move(c, id, id+deltas[i]);
                    travel--;
                    goto travel_start;
                }
            }
            goto unsettled;
        }

        goto settled;
    }
    
    // Add the acceleration due to gravity
    velocity = CELL_VELOCITY(c->mesh[id].data);
    if (velocity < 15){
        CELL_VELOCITY_SET(c->mesh[id].data, velocity+1);
    }

    c->moved[id] = 1;
    return;


settled:
    CELL_VELOCITY_CLR(c->mesh[id].data);
    SET_FLAG(c->mesh[id].data, CELL_SETTLED_FLAG);
unsettled:
    // Stop, settle and mark the cell as not moved
    c->moved[id] = 0;
    return;
}


void set_cell(struct chunk *c, size_t id, uint16_t kind, uint16_t data){
    c->flags = 1;

    c->mesh[id].kind = kind;
    c->mesh[id].data = data;

    if (id < CHUNK_AREA-CHUNK_SIZE){
        CLR_FLAG(c->mesh[id + off_u].data, CELL_SETTLED_FLAG); 
        CLR_FLAG(c->mesh[id + off_ul].data, CELL_SETTLED_FLAG); 
        CLR_FLAG(c->mesh[id + off_ur].data, CELL_SETTLED_FLAG);
    }
    if (id < CHUNK_AREA){
        CLR_FLAG(c->mesh[id + off_r].data, CELL_SETTLED_FLAG);
    }
    if (id > 0){
        CLR_FLAG(c->mesh[id + off_l].data, CELL_SETTLED_FLAG);
    }
    if (id > CHUNK_SIZE){
        CLR_FLAG(c->mesh[id + off_d].data, CELL_SETTLED_FLAG);
        CLR_FLAG(c->mesh[id + off_dr].data, CELL_SETTLED_FLAG);
        CLR_FLAG(c->mesh[id + off_dl].data, CELL_SETTLED_FLAG);
    }

}


