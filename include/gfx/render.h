#pragma once
#include <math.h>
#include "shader.h"
#include "../sim/sim.h"
#include "../common/common.h"
#include "../common/utils.h"
#include "render/cell_renderer.h"

#define SHADER_CELL_VERT "assets/cell_vert.glsl"
#define SHADER_CELL_FRAG "assets/cell_frag.glsl"

enum renderer_type {
    CellRenderer,
    DebugRenderer,
};

vec2 *render_camera();

int render_draw();
int render_init();
void render_update_chunks(struct universe *c);
