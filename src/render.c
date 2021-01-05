#include "../include/render.h"

struct renderer {
    int initialized;
    GLuint *programs;
    GLuint *vert_arrays;
    GLuint *buffers;
    GLuint *textures;
    GLuint *vertex_counts;
    GLuint *uniform_locs;
};

// Cell renderer handles rendering the particle simulation
static struct renderer cell_renderer = { .initialized = 0 };
struct renderer *renderer_cell(){ return &cell_renderer; }

// Debug renderer handles debug overlays
static struct renderer debug_renderer = { .initialized = 0 };
struct renderer *renderer_debug() { return &debug_renderer; }

static struct render_chunk screen_chunks[NUM_SCREEN_CHUNKS];

int init_cell_renderer(){
    struct renderer *r = &cell_renderer;
    size_t num_kinds = sizeof(kinds)/sizeof(struct kind_property);
    uint32_t colors[num_kinds];
    
    // Allocate required memory
    r->programs = calloc(1, sizeof(GLuint));
    r->vert_arrays = calloc(1, sizeof(GLuint));
    r->buffers = calloc(2, sizeof(GLuint));
    r->vertex_counts = calloc(1, sizeof(GLuint));
    r->uniform_locs = calloc(2, sizeof(GLuint));
    
    // Ensure memory was allocated successfully
    if (!r->programs || !r->vert_arrays || !r->buffers 
        || !r->vertex_counts || !r->uniform_locs){
        fprintf(stderr, "Failed to allocate memory for cell renderer\n");
        goto err;
    }
    
    r->initialized = 1;
    r->vertex_counts[0] = 0;
    
    // Compile and link shaders
    if (!shader_program_from_files(&r->programs[0], SHADER_CELL_VERT,
                                                    SHADER_CELL_FRAG)) {
        fprintf( stderr, "Failed to build cell renderer program\n");
        goto err;
    }
    
    glUseProgram(r->programs[0]);
    
    // Assign constant uniforms
    GLuint ch_size_loc = glGetUniformLocation(r->programs[0], "chunk_size");
    GLuint pt_size_loc = glGetUniformLocation(r->programs[0], "point_size");
    glUniform1ui(ch_size_loc, CHUNK_SIZE);
    glUniform1ui(pt_size_loc, POINT_SIZE);
    // Locate dynamic uniforms
    r->uniform_locs[0] = glad_glGetUniformLocation(r->programs[0],"mvp");
    r->uniform_locs[1] = glad_glGetUniformLocation(r->programs[0],"chunk_pos");
    
    // Generate buffers and array objects
    glGenVertexArrays(1, r->vert_arrays);
    glGenBuffers(2, r->buffers);

    glBindVertexArray(r->vert_arrays[0]);
    
    // Generate the color lookup for the frag shader storage buffer
    for (size_t i = 0; i < num_kinds; i++){
        colors[i] = kinds[i].color;
    }
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, r->buffers[1]);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(colors), 
                 colors, GL_STATIC_COPY);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, r->buffers[1]); 
     
    // Allocate an empty buffer large enough to hold all screen chunk data
    glBindBuffer(GL_ARRAY_BUFFER, r->buffers[0]);
    glBufferStorage(GL_ARRAY_BUFFER, 
                    sizeof(int32_t) * CHUNK_AREA * NUM_SCREEN_CHUNKS,
                    NULL,
                    GL_DYNAMIC_STORAGE_BIT);
    // Unbind the renderer from OpenGL
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
    glBindVertexArray(0);
    return 1;

err:
    return 0;
}

int renderer_load_chunk(struct chunk *sim_chunk){
    if (!sim_chunk->render_chunk){
        fprintf(stderr, "Cannot render a chunk that is not set to rendered\n");
        goto err;
    }

    struct renderer *r = &cell_renderer;
    struct render_chunk *rchunk = sim_chunk->render_chunk; 
    r->vertex_counts[0] = rchunk->cell_count;

    glUseProgram(r->programs[0]);
    glBindVertexArray(r->vert_arrays[0]);
    glBindBuffer(GL_ARRAY_BUFFER, r->buffers[0]);
     
    glBufferSubData(GL_ARRAY_BUFFER, 0, 
                    sizeof(struct render_cell) * rchunk->cell_count, 
                    rchunk->cells);

    glVertexAttribIPointer(0, 1, GL_UNSIGNED_INT, sizeof(GLuint), (void*) 0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);
    return 1;
err:
    return 0;
}


void renderer_update_chunks(struct universe *u){
    if (!u->grid[0].render_chunk){
        chunk_set_rendered(&u->grid[0], &screen_chunks[0]);
    }
    
    renderer_load_chunk(&u->grid[0]); 
}


int renderer_draw(enum renderer_type rtype) {
    switch (rtype){
        case CellRenderer: {
            struct renderer *r = &cell_renderer;
            
            // Bind required objects
            glUseProgram(r->programs[0]);
            glBindVertexArray(r->vert_arrays[0]);
            glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, r->buffers[1]); 
            
            for (int i = 0; i < NUM_SCREEN_CHUNKS; i++){
                // Generate MVP for this chunk
                float ortho[16];
                linalg_ortho(ortho, 0, RESOLUTION_X, 0, RESOLUTION_Y);
                glUniformMatrix4fv(r->uniform_locs[0], 1, GL_FALSE, ortho);
                
                // TODO: use sim_chunk pos
                // Calculate this chunk's pos
                int chunk_pos[] = { i % SCREEN_CHUNKS_X, 
                                    i / SCREEN_CHUNKS_X };
                
                glUniform2iv(r->uniform_locs[1], 1, chunk_pos);
                glDrawArrays(GL_POINTS, 0, r->vertex_counts[0]);            
            }
            
            glBindVertexArray(0);
            break;
        }
        
        default:
            break;
    }
    return 1;
}


void render_destroy() {
    if (cell_renderer.initialized){
        free(cell_renderer.programs);
        free(cell_renderer.vert_arrays);
        free(cell_renderer.buffers);
        free(cell_renderer.vertex_counts);
        free(cell_renderer.uniform_locs);
        cell_renderer.initialized = 0;
    }

    if (debug_renderer.initialized){
        free(debug_renderer.programs);
        free(debug_renderer.vert_arrays);
        free(debug_renderer.buffers);
        debug_renderer.initialized = 0;
    }
}

int render_init() {
    if (!init_cell_renderer()){
        fprintf(stderr, "Failed to init cell renderer.\n");
        goto err;
    }

    return 1;
err:
    render_destroy();
    return 0;
}
