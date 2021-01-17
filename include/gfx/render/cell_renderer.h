#pragma once

#include <math.h>
#include "../shader.h"
#include "../../sim/sim.h"
#include "../../common/common.h"
#include "../../common/utils.h"

#define SHADER_CELL_VERT "assets/cell_vert.glsl"
#define SHADER_CELL_FRAG "assets/cell_frag.glsl"

int cell_renderer_init(vec2 *camera, struct render_chunk *scr_chunks);
int cell_renderer_load_chunk(struct chunk *sim_chunk);
int cell_renderer_draw();
