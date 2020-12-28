#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include "glad/glad.h"
#include "common.h"
#include "sim.h"


static const float SCREEN_QUAD[16] = {
     1.0,  1.0, 1.0, 1.0,
     1.0, -1.0, 1.0, 0.0,
    -1.0, -1.0, 0.0, 0.0,
    -1.0,  1.0, 0.0, 1.0
};

static const unsigned int SCREEN_QUAD_IND[6] = {
    0, 1, 3,
    1, 2, 3
};

struct render_data {
    GLuint gl_program;
    GLuint gl_vbo;
    GLuint gl_ebo;
    GLuint gl_vao;
    GLuint gl_tex;
};


void renderer_load_chunk(struct render_data *r, struct chunk *c); 
int renderer_init(struct render_data *r, GLuint program);
void renderer_draw(struct render_data *r);

