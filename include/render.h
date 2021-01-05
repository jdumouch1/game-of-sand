#pragma once
#include "shader.h"
#include "sim.h"
#include "common.h"

#define SHADER_CELL_VERT "assets/cell_vert.glsl"
#define SHADER_CELL_FRAG "assets/cell_frag.glsl"

enum renderer_type {
    CellRenderer,
    DebugRenderer,
};

int renderer_draw(enum renderer_type rtype);
int render_init();
void render_destroy();

void renderer_update_chunks(struct universe *c);
