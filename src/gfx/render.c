#include "../../include/gfx/render.h"

static mat4 projection;
static vec2 camera = { .x = 0, .y = 0 };

static struct render_chunk screen_chunks[NUM_SCREEN_CHUNKS];
vec2 *render_camera(){ return &camera; }

int render_load_chunk(struct chunk *sim_chunk){
    if (!cell_renderer_load_chunk(sim_chunk)){
        goto err;
    }

    return 1;
err:
    return 0;
}


void render_update_chunks(struct universe *u){
    for (int i = 0; i < NUM_SCREEN_CHUNKS; i++){
        if (!u->grid[i].render_chunk){
            chunk_set_rendered(&u->grid[i], &screen_chunks[i]);
        }
        render_load_chunk(&u->grid[i]); 
    }
}

int render_draw(){
    return cell_renderer_draw();
}


int render_init() {
    if (!cell_renderer_init(&camera, screen_chunks)){
        fprintf(stderr, "Failed to init cell renderer.\n");
        goto err;
    }

    return 1;

err:
    return 0;
}
