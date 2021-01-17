#include "../../../include/gfx/render/cell_renderer.h"
struct cell_renderer {
    int initialized;
    mat4 projection;
    GLuint program;
    GLuint vert_array;
    GLuint buffers[2];
    GLuint uniform_locs[2];
};

// Cell renderer handles rendering the particle simulation
static struct cell_renderer renderer = {0};
static struct render_chunk *screen_chunks;
static vec2 *camera;

int cell_renderer_init(vec2 *camera_inst, struct render_chunk *scr_chunks){
    camera = camera_inst;
    screen_chunks = scr_chunks;

    struct cell_renderer *cr = &renderer;
    *cr = (struct cell_renderer) {0};
    cr->projection = lalg_ortho(0, RESOLUTION_X, 0, RESOLUTION_Y);

    size_t num_kinds = sizeof(kinds)/sizeof(struct kind_property);
    uint32_t colors[num_kinds];
    
    // Compile and link shaders
    if (!shader_program_from_files(&cr->program, SHADER_CELL_VERT,
                                                 SHADER_CELL_FRAG)) {
        fprintf( stderr, "Failed to build cell renderer program\n");
        goto err;
    }
    
    glUseProgram(cr->program);
    
    // Assign constant uniforms
    GLuint ch_size_loc = glGetUniformLocation(cr->program, "chunk_size");
    GLuint pt_size_loc = glGetUniformLocation(cr->program, "point_size");
    glUniform1ui(ch_size_loc, CHUNK_SIZE);
    glUniform1ui(pt_size_loc, POINT_SIZE);
    // Locate dynamic uniforms
    cr->uniform_locs[0] = glGetUniformLocation(cr->program, "mvp");
    cr->uniform_locs[1] = glGetUniformLocation(cr->program, "chunk_pos");
    
    // Generate buffers and array objects
    glGenVertexArrays(1, &cr->vert_array);
    glGenBuffers(2, cr->buffers);

    glBindVertexArray(cr->vert_array);
    
    // Generate the color lookup for the frag shader storage buffer
    for (size_t i = 0; i < num_kinds; i++){
        colors[i] = kinds[i].color;
    }
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, cr->buffers[1]);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(colors), 
                 colors, GL_STATIC_COPY);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, cr->buffers[1]); 
     
    // Allocate an empty buffer large enough to hold all screen chunk data
    glBindBuffer(GL_ARRAY_BUFFER, cr->buffers[0]);
    glBufferStorage(GL_ARRAY_BUFFER, 
                    sizeof(struct render_cell) * 
                    CHUNK_AREA * NUM_SCREEN_CHUNKS,
                    NULL,
                    GL_DYNAMIC_STORAGE_BIT);
    // Unbind the renderer from OpenGL
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
    glBindVertexArray(0);
    return 1;

err:
    return 0;
}

int cell_renderer_load_chunk(struct chunk *sim_chunk){
    if (!sim_chunk->render_chunk){
        fprintf(stderr, "Cannot render a chunk that is not set to rendered\n");
        goto err;
    }

    struct cell_renderer *r = &renderer;
    struct render_chunk *rchunk = sim_chunk->render_chunk; 
    size_t rchunk_id = rchunk - screen_chunks; 
    glUseProgram(r->program);
    glBindVertexArray(r->vert_array);
    glBindBuffer(GL_ARRAY_BUFFER, r->buffers[0]);
     
    glBufferSubData(GL_ARRAY_BUFFER, 
                    sizeof(struct render_cell) * CHUNK_AREA * rchunk_id, 
                    sizeof(struct render_cell) * rchunk->cell_count, 
                    rchunk->cells);

    glVertexAttribIPointer(0, 1, GL_UNSIGNED_INT, sizeof(GLuint), (void*) 0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);
    return 1;
err:
    return 0;
}

int cell_renderer_draw(){
    struct cell_renderer *r = &renderer;
    
    // Bind required objects
    glUseProgram(r->program);
    glBindVertexArray(r->vert_array);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, r->buffers[1]); 
   
    // Generate view-projection matrix 
    mat4 view = lalg_translation(camera->x, camera->y, 0.0);
    mat4 vp = lalg_cross(&r->projection, &view);
    glUniformMatrix4fv(r->uniform_locs[0], 1, GL_TRUE, vp.m);

    for (int i = 0; i < NUM_SCREEN_CHUNKS; i++){
        // Try not to draw the chunk
        if (!screen_chunks[i].cell_count) { 
            if (CHK_FLAG(screen_chunks[i].flags, RCHUNK_DREW_EMPTY)){
                continue; 
            }
            else{
                // Allow the chunk one frame to draw nothing 
                SET_FLAG(screen_chunks[i].flags, RCHUNK_DREW_EMPTY); 
            }
        } 
        // If we are drawing, clear the "drew empty" flag
        else { CLR_FLAG(screen_chunks[i].flags, RCHUNK_DREW_EMPTY); }

        int chunk_pos[] = { screen_chunks[i].sim_chunk->pos.x, 
                            screen_chunks[i].sim_chunk->pos.y };

        glUniform2iv(r->uniform_locs[1], 1, chunk_pos);
        glDrawArrays(GL_POINTS, i * CHUNK_AREA, 
                     screen_chunks[i].cell_count);            
    }
    
    glBindVertexArray(0);
    return 1;
}


