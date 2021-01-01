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

inline int vec2_to_id(struct vec2 *v){
    return v->x + (v->y << CHUNK_SCALE);
}

inline void id_to_vec2(struct vec2 *out, int id){
    out->x = id % CHUNK_SIZE;
    out->y = id >> CHUNK_SCALE;
}

// https://stackoverflow.com/questions/664852/which-is-the-fastest-way-to-get-the-absolute-value-of-a-number
// by vicatu
int32_t fast_abs(int32_t a){
    uint32_t tmp = (a>>31);
    return (a^tmp) + (tmp & 1); 
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
            && !CHK_FLAG(cell->flags, CELL_SETTLED_FLAG)
            && !CHK_FLAG(cell->flags, CELL_STATIC_FLAG)){
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
    
    // density less than to cell?
    uint8_t from_density = kinds[c->mesh[id].kind].density; 
    uint8_t to_density = kinds[c->mesh[to].kind].density;
    if (to_density >= from_density){
        can_move = 0;
        goto out;
    }
    // to cell has been updated already?
    else if (c->moved[to]){
        can_move = 0;
        goto out;
    }
 
    // trying to move off edge of cell ?
    int is_left = delta/CHUNK_SIZE != 0;
    if ((is_left && (id%CHUNK_SIZE) == 0) 
        || (!is_left && (id%CHUNK_SIZE)==CHUNK_SIZE-1)){
        can_move = 0;
        goto out;
    }

out:
    return can_move;
}

void unsettle_neighbours(struct chunk *c, size_t id){
    if (id < CHUNK_AREA-CHUNK_SIZE){
        CLR_FLAG(c->mesh[id + off_u].flags, CELL_SETTLED_FLAG); 
        CLR_FLAG(c->mesh[id + off_ul].flags, CELL_SETTLED_FLAG); 
        CLR_FLAG(c->mesh[id + off_ur].flags, CELL_SETTLED_FLAG);
    }
    if (id < CHUNK_AREA){
        CLR_FLAG(c->mesh[id + off_r].flags, CELL_SETTLED_FLAG);
    }
    if (id > -1){
        CLR_FLAG(c->mesh[id + off_l].flags, CELL_SETTLED_FLAG);
    }
    if (id > CHUNK_SIZE){
        CLR_FLAG(c->mesh[id + off_d].flags, CELL_SETTLED_FLAG);
        CLR_FLAG(c->mesh[id + off_dr].flags, CELL_SETTLED_FLAG);
        CLR_FLAG(c->mesh[id + off_dl].flags, CELL_SETTLED_FLAG);
    }
}

size_t cell_move(struct chunk *c, size_t from, int delta){
    struct cell *cell = &c->mesh[from];
    struct cell tmp = c->mesh[from];
    struct cell *new_cell = &c->mesh[from+delta];
    if (cell->kind == new_cell->kind){}
    memcpy(cell, new_cell, sizeof(struct cell));
    memcpy(new_cell, &tmp, sizeof(struct cell));
    cell = new_cell;

    unsettle_neighbours(c, from);

    return from+delta;
}

void cell_update(struct chunk *c, size_t id){
    int32_t vx = c->mesh[id].vx;
    int vy = c->mesh[id].vy;
    int xtravel = abs(vx);
    int ytravel = abs(vy) + 1;
    size_t start_id = id;
    while (ytravel){
        // Down
        if (cell_can_move(c, id, off_d)){
            id = cell_move(c, id, off_d);
            goto moved;
        }else if (id > CHUNK_SIZE){
            if (!CHK_FLAG(c->mesh[id+off_d].flags, CELL_SETTLED_FLAG)){
                c->mesh[id+off_d].vy = min(vy, c->mesh[id+off_d].vy);
            }
        }

        // Deflection
        int delta[2] = { off_dl, off_dr };
        int choice = rand() % 2;
        int friction_odds = rand() % 255;
        for (int i = 0; i < 2; i++){
            choice = (choice+1)%2;
            if (cell_can_move(c, id, delta[choice])){
                if (friction_odds < kinds[c->mesh[id].kind].friction) { 
                    goto stopped; 
                }

                id = cell_move(c, id, delta[choice]);
                goto moved;
            }
        }

        goto settled;
    moved: 
        ytravel--;
    }

stopped:
    if (id == start_id) { goto settled; }
    c->mesh[id].vy--;
    c->moved[id] = 1;
    return;
settled:
    SET_FLAG(c->mesh[id].flags, CELL_SETTLED_FLAG);
    c->moved[0] = 1;
    c->mesh[id].vy = 0;
}


void set_cell(struct chunk *c, size_t id, struct cell new_cell){
    c->flags = 1;
    c->mesh[id] = new_cell;
    unsettle_neighbours(c, id);
}


