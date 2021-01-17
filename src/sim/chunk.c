#include "../../include/sim/chunk.h"
int chunk_update_cell(struct chunk *c, size_t *id);

void chunk_set_rendered(struct chunk *c, struct render_chunk *rc){
    if (rc->sim_chunk){
        // Decouple the old chunk
        CLR_FLAG(rc->sim_chunk->flags, CHUNK_RENDERED);
        rc->sim_chunk->render_chunk = NULL;
    }
    memset(rc, 0, sizeof(struct render_chunk));

    // Couple the new chunk
    c->render_chunk = rc;
    rc->sim_chunk = c;
    
    SET_FLAG(c->flags, CHUNK_RENDERED);
}

void chunk_update(struct chunk *c){
    // Reset the cell updated states
    //memset(&c->moved, 0, sizeof(c->moved));
    // Set the chunk to inactive, in case no cells are updated.
    CLR_FLAG(c->flags, CHUNK_ACTIVE);
    for (int i = 0; i < CHUNK_AREA; i++){
        CLR_FLAG(c->mesh[i].flags, CELL_UPDATED);
    }

    int flipped_update = CHK_FLAG(c->flags, CHUNK_FLIP_UPDATE);

    int incrementor = flipped_update ? -1 : 1;
    size_t i = flipped_update * (CHUNK_SIZE-1);
    if (c->render_chunk){
        memset(c->render_chunk->cells, 0, sizeof(c->render_chunk->cells));
        c->render_chunk->cell_count = 0;
    }
    while (i < CHUNK_AREA){
        struct cell *cell = &c->mesh[i];
        size_t id = i;
        
        // Try to update the cell
        if (!CHK_FLAG(c->mesh[i].flags, CELL_UPDATED)
            && c->mesh[i].kind
//            && !CHK_FLAG(cell->flags, CELL_SETTLED)
            && !CHK_FLAG(cell->flags, CELL_STATIC)){
            
            chunk_update_cell(c, &id);
            
            // Set that has been an update in this chunk
            SET_FLAG(c->flags, CHUNK_ACTIVE);
        }
        
        // Add to render_chunk if this chunk is on screen
        if (CHK_FLAG(c->flags, CHUNK_RENDERED) &&
            c->mesh[id].kind){
            struct render_chunk *rc = c->render_chunk;
            rc->cells[rc->cell_count++] = (struct render_cell) {
                .local_id = id,
                .kind = c->mesh[id].kind,
            };
        }
        
        // jump up a row if flipped and at start of row
        if (flipped_update && !(i % CHUNK_SIZE)){
            i = ((i/CHUNK_SIZE)+1)*CHUNK_SIZE + CHUNK_SIZE;
        } 
        i += incrementor;
    }

    TGL_FLAG(c->flags, CHUNK_FLIP_UPDATE);
}


int chunk_can_cell_move(struct chunk *c, size_t from, int dx, int dy){
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


size_t chunk_move_cell(struct chunk *c, size_t from, int dx, int dy){
    struct cell *cell = &c->mesh[from];
    struct cell tmp = c->mesh[from];
    struct cell *new_cell = &c->mesh[from + dx + (dy<<CHUNK_SCALE)];
    if (cell->kind == new_cell->kind){}
    memcpy(cell, new_cell, sizeof(struct cell));
    memcpy(new_cell, &tmp, sizeof(struct cell));
    cell = new_cell;

//    unsettle_neighbours(c, from);
    
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

int chunk_update_cell(struct chunk *c, size_t *id){
    if (!c->mesh[*id].kind){
        SET_FLAG(c->mesh[*id].flags, CELL_SETTLED);
        goto stopped;
    }

    int travel = c->mesh[*id].speed>>3;
    if (!travel) {
        travel++;
        c->mesh[*id].speed++;
        return 0;
    }
    
    const struct kind_property props = kinds[c->mesh[*id].kind];

    int dir = (props.density < 0) - (props.density > 0);

    SET_FLAG(c->mesh[*id].flags, CELL_FALLING);

    while (travel){
        if (!move_in_bounds(*id, 0, dir)){ goto out_of_bounds; }
        if (chunk_can_cell_move(c, *id, 0, dir)){
            *id = chunk_move_cell(c, *id, 0, dir);
            goto moved;
        }
        CLR_FLAG(c->mesh[*id].flags, CELL_FALLING); 

        // Try diagonal movement towards the bias
        int bias = CHK_FLAG(c->mesh[*id].flags, CELL_BIAS) ? 1 : -1;
        if (!move_in_bounds(*id, bias, dir)){ goto out_of_bounds; }
        if (chunk_can_cell_move(c, *id, bias, dir)){
            *id = chunk_move_cell(c, *id, bias, dir);
            goto moved;
        }
        // Otherwise try the other diagonal, and flip the bias
        else{
            if (!move_in_bounds(*id, -bias, -1)){ goto out_of_bounds; }
            if (chunk_can_cell_move(c, *id, -bias, dir)){
                *id = chunk_move_cell(c, *id, -bias, dir);

                // Flip the bias (needed for dispersion)
                TGL_FLAG(c->mesh[*id].flags, CELL_BIAS);
                bias = CHK_FLAG(c->mesh[*id].flags, CELL_BIAS) ? 1 : -1;
                
                goto moved;
             }
        }
        

        // If the cell does not have dispersion its stopped
        if (!props.dispersion){ 
            if (CHK_FLAG(c->mesh[(*id)+dir*CHUNK_SIZE].flags, CELL_STATIC) 
                || CHK_FLAG(c->mesh[(*id)+dir*CHUNK_SIZE].flags, CELL_SETTLED)){
                goto settled;
            }

            return 0;
        }

        // Otherwise try to disperse horizontally
        if (!move_in_bounds(*id, bias, -1)){ goto out_of_bounds; }
        if (chunk_can_cell_move(c, *id, bias, 0)){
            goto disperse;
        }

        // Try to flip the bias and disperse the other way
        else {
            if (!move_in_bounds(*id, -bias, 0)) { goto out_of_bounds; }
            if (chunk_can_cell_move(c, *id, -bias, 0)){
                bias = -bias;
                TGL_FLAG(c->mesh[*id].flags, CELL_BIAS);
                goto disperse;
            }
        }
        goto settled; 
        //if (rand()%1024 > props.dispersion){goto stopped;}
        return 0;
    disperse:
        if (rand() % 255 >= props.dispersion) { goto moved; }
        c->mesh[*id].speed = 0;
        *id = chunk_move_cell(c, *id, bias, 0);
        //goto moved;
        c->mesh[*id].speed+=20;
        travel--;
        continue;
    moved:
        travel--;
        continue;
    } 
    
    c->mesh[*id].speed = min(MAX_SPEED, c->mesh[*id].speed+GRAVITY_ACCEL);
    SET_FLAG(c->mesh[*id].flags, CELL_UPDATED);
    CLR_FLAG(c->mesh[*id].flags, CELL_SETTLED);
    return 1;

stopped:
    //c->mesh[id].speed = props.dispersion;
    return 0;

settled:
    //c->mesh[id].speed = props.dispersion>>2;
    SET_FLAG(c->mesh[*id].flags, CELL_SETTLED);
    return 0;

out_of_bounds:
    c->mesh[*id] = EMPTY_CELL;
    return 1;
}

void chunk_set_cell(struct chunk *c, size_t id, struct cell new_cell){
    SET_FLAG(c->flags, CHUNK_ACTIVE);
    c->mesh[id] = new_cell;
    //unsettle_neighbours(c, id); 
}
