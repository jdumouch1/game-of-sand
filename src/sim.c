#include "../include/sim.h"
#include <bits/stdint-uintn.h>

int cell_update(struct chunk *c, size_t id);

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

void chunk_update(struct chunk *c){
    // Reset the cell updated states
    memset(&c->moved, 0, sizeof(c->moved));
    // Set the chunk to inactive, in case no cells are updated.
    CLR_FLAG(c->flags, CHUNK_ACTIVE);
    
    int flipped_update = CHK_FLAG(c->flags, CHUNK_FLIP_UPDATE);

    int incrementor = flipped_update ? -1 : 1;
    size_t i = flipped_update * (CHUNK_SIZE-1);
    c->dirt_rect_corners.x = CHUNK_AREA-1;
    c->dirt_rect_corners.y = 0;
    while (i < CHUNK_AREA){
        struct cell *cell = &c->mesh[i];
        

        if (!c->moved[i]
            && c->mesh[i].kind
            && !CHK_FLAG(cell->flags, CELL_SETTLED)
            && !CHK_FLAG(cell->flags, CELL_STATIC)){
            
            uvec2 pos = { .x = i % CHUNK_SIZE, .y = i >> CHUNK_SCALE };
            uvec2 bl = { .x = c->dirt_rect_corners.x % CHUNK_SIZE, 
                         .y = c->dirt_rect_corners.x >> CHUNK_SCALE };
            uvec2 tr = { .x = c->dirt_rect_corners.y % CHUNK_SIZE,
                         .y = c->dirt_rect_corners.y >> CHUNK_SCALE };
            if (cell_update(c, i)){
                if (bl.x > pos.x){
                    bl.x = pos.x;
                }
                if (bl.y > pos.y){
                    bl.y = pos.y;
                }
                if (tr.x < pos.x){
                    tr.x = pos.x;
                }
                if (tr.y < pos.y){
                    tr.y = pos.y;
                }

                c->dirt_rect_corners.x = bl.x + (bl.y << CHUNK_SCALE);
                c->dirt_rect_corners.y = tr.x + (tr.y << CHUNK_SCALE);
            }
            
            // Set that has been an update in this chunk
            SET_FLAG(c->flags, CHUNK_ACTIVE);
        }

        // Check if 'i' needs to move to the end of a higher row
    //printf("Moved\n");
        if (flipped_update && !(i % CHUNK_SIZE)){
            // (Current Row + 1) * Row_size = start of next row
            // row_start + row_size = (end of next row+1)
            // The extra +1 is decremented this iteration
            i = ((i/CHUNK_SIZE)+1)*CHUNK_SIZE + CHUNK_SIZE;
        } 
        i += incrementor;
    }

    TGL_FLAG(c->flags, CHUNK_FLIP_UPDATE);
}

void unsettle_neighbours(struct chunk *c, size_t id){
    if (id < CHUNK_AREA-CHUNK_SIZE){
        CLR_FLAG(c->mesh[id + off_u].flags, CELL_SETTLED); 
        CLR_FLAG(c->mesh[id + off_ul].flags, CELL_SETTLED); 
        CLR_FLAG(c->mesh[id + off_ur].flags, CELL_SETTLED);
    }
    if (id < CHUNK_AREA){
        CLR_FLAG(c->mesh[id + off_r].flags, CELL_SETTLED);
    }
    if (id > -1){
        CLR_FLAG(c->mesh[id + off_l].flags, CELL_SETTLED);
    }
    if (id > CHUNK_SIZE){
        CLR_FLAG(c->mesh[id + off_d].flags, CELL_SETTLED);
        CLR_FLAG(c->mesh[id + off_dr].flags, CELL_SETTLED);
        CLR_FLAG(c->mesh[id + off_dl].flags, CELL_SETTLED);
    }
}

int cell_can_move_by(struct chunk *c, size_t from, int dx, int dy){
    size_t to_id = from + dx + (dy << CHUNK_SCALE);
    struct kind_property from_props = kinds[c->mesh[from].kind];
    struct kind_property to_props = kinds[c->mesh[to_id].kind];

    if (!c->mesh[to_id].kind) {
        return 1;
    }

    if (from_props.density > to_props.density 
        && !CHK_FLAG(c->mesh[to_id].flags, CELL_STATIC)){
        return 1;
    }

    return 0;
}


size_t cell_move_by(struct chunk *c, size_t from, int dx, int dy){
    struct cell *cell = &c->mesh[from];
    struct cell tmp = c->mesh[from];
    struct cell *new_cell = &c->mesh[from + dx + (dy<<CHUNK_SCALE)];
    if (cell->kind == new_cell->kind){}
    memcpy(cell, new_cell, sizeof(struct cell));
    memcpy(new_cell, &tmp, sizeof(struct cell));
    cell = new_cell;

    unsettle_neighbours(c, from);
    
    return from + dx + (dy<<CHUNK_SCALE);
}




int move_in_bounds(size_t id, int dx, int dy){
    int in_bounds = 1;
    vec2 pos = { .x = id % CHUNK_SIZE, .y = id / CHUNK_SIZE };
    if (dx && (pos.x+dx < 0 || pos.x+dx >= CHUNK_SIZE)){
        in_bounds = 0;
        goto out;
    }
    
    if (dy && (pos.y+dy < 0 || pos.y+dy >= CHUNK_SIZE)) {
        in_bounds = 0;
        goto out;
    }

out:
    return in_bounds;
}

int cell_update(struct chunk *c, size_t id){
    if (!c->mesh[id].kind){
        SET_FLAG(c->mesh[id].flags, CELL_SETTLED);
        goto stopped;
    }

    int travel = c->mesh[id].speed>>3;
    if (!travel) {
        travel++;
        c->mesh[id].speed++;
        return 0;
    }
    
    vec2 pos = { .x = id%CHUNK_SIZE, .y = id >> CHUNK_SCALE };
    const struct kind_property props = kinds[c->mesh[id].kind];

    int dir = (props.density < 0) - (props.density > 0);

    SET_FLAG(c->mesh[id].flags, CELL_FALLING);

    while (travel){
        if (!move_in_bounds(id, 0, dir)){ goto out_of_bounds; }
        if (cell_can_move_by(c, id, 0, dir)){
            id = cell_move_by(c, id, 0, dir);
            goto moved;
        }
        CLR_FLAG(c->mesh[id].flags, CELL_FALLING); 

        // Try diagonal movement towards the bias
        int bias = CHK_FLAG(c->mesh[id].flags, CELL_BIAS) ? 1 : -1;
        if (!move_in_bounds(id, bias, dir)){ goto out_of_bounds; }
        if (cell_can_move_by(c, id, bias, dir)){
            id = cell_move_by(c, id, bias, dir);
            goto moved;
        }
        // Otherwise try the other diagonal, and flip the bias
        else{
            if (!move_in_bounds(id, -bias, -1)){ goto out_of_bounds; }
            if (cell_can_move_by(c, id, -bias, dir)){
                id = cell_move_by(c, id, -bias, dir);

                // Flip the bias (needed for dispersion)
                TGL_FLAG(c->mesh[id].flags, CELL_BIAS);
                bias = CHK_FLAG(c->mesh[id].flags, CELL_BIAS) ? 1 : -1;
                
                goto moved;
             }
        }
        

        // If the cell does not have dispersion its stopped
        if (!props.dispersion){ 
            if (CHK_FLAG(c->mesh[id+dir*CHUNK_SIZE].flags, CELL_STATIC) 
                || CHK_FLAG(c->mesh[id+dir*CHUNK_SIZE].flags, CELL_SETTLED)){
                goto settled;
            }

            return 0;
        }

        // Otherwise try to disperse horizontally
        if (!move_in_bounds(id, bias, -1)){ goto out_of_bounds; }
        if (cell_can_move_by(c, id, bias, 0)){
            goto disperse;
        }

        // Try to flip the bias and disperse the other way
        else {
            if (!move_in_bounds(id, -bias, 0)) { goto out_of_bounds; }
            if (cell_can_move_by(c, id, -bias, 0)){
                bias = -bias;
                TGL_FLAG(c->mesh[id].flags, CELL_BIAS);
                goto disperse;
            }
        }
        goto settled; 
        //if (rand()%1024 > props.dispersion){goto stopped;}
        return 0;
    disperse:
        if (rand() % 255 >= props.dispersion) { goto moved; }
        c->mesh[id].speed = 0;
        id = cell_move_by(c, id, bias, 0);
        //goto moved;
        c->mesh[id].speed+=20;
        travel--;
        continue;
    moved:
        travel--;
        continue;
    } 
    
    c->mesh[id].speed = min(MAX_SPEED, c->mesh[id].speed+GRAVITY_ACCEL);
    c->moved[id] = dir > 0;
    CLR_FLAG(c->mesh[id].flags, CELL_SETTLED);
    CLR_FLAG(c->mesh[id].flags, CELL_IDLED);
    return 1;

stopped:
    //c->mesh[id].speed = props.dispersion;
    return 0;

settled:
    //c->mesh[id].speed = props.dispersion>>2;
    SET_FLAG(c->mesh[id].flags, CELL_SETTLED);
    return 0;

out_of_bounds:
    c->mesh[id] = EMPTY_CELL;
    return 1;
}

void set_cell(struct chunk *c, size_t id, struct cell new_cell){
    SET_FLAG(c->flags, CHUNK_ACTIVE);
    c->mesh[id] = new_cell;
    unsettle_neighbours(c, id); 
}


