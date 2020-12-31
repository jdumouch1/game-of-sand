#include "../include/render.h"

static enum render_mode current_mode = render_kinds;
void renderer_set_mode(enum render_mode r){
    current_mode = r;
}
enum render_mode renderer_get_mode(){
    return current_mode;
}

void renderer_load_chunk(struct render_data *r, struct chunk *c){
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, r->gl_tex);
    
    GLubyte *pixels = calloc(CHUNK_AREA * 4, sizeof(GLubyte));
    for (size_t i = 0; i<CHUNK_AREA * 4; i+=4){
        uint8_t r_channel, g_channel, b_channel;
        switch (current_mode) {
        case render_kinds:
            {
                uint32_t color = kinds[c->mesh[i/4].kind].color;
                r_channel = (color >> 16);
                g_channel = (color >>  8) & 0xFF;
                b_channel = (color >>  0) & 0xFF;
            }
            break;
        case render_flags:
            {
                uint8_t settled = CHK_FLAG(c->mesh[i/4].data, 
                                           CELL_SETTLED_FLAG);
                uint8_t static_ = CHK_FLAG(c->mesh[i/4].data, 
                                           CELL_STATIC_FLAG);
                r_channel = (!settled)*255;
                g_channel = static_*255;
                b_channel = 0;
            }
            break;
        case render_density:
            {
                uint8_t density = kinds[c->mesh[i/4].kind].density;
                r_channel = density;
                g_channel = density;
                b_channel = density;
            }
            break;
        case render_velocity:
            {
                uint8_t vel = CELL_VELOCITY(c->mesh[i/4].data);
                r_channel = vel * (255/15);
                g_channel = 0;
                b_channel = 0;
            }
            break;
        default:
            r_channel = 0;
            g_channel = 0;
            b_channel = 0;
            break;
        }
        pixels[i+0] = r_channel;
        pixels[i+1] = g_channel;
        pixels[i+2] = b_channel;
        pixels[i+3] = 255;
}

    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, CHUNK_SIZE, CHUNK_SIZE, 
                    GL_RGBA, GL_UNSIGNED_BYTE, pixels);

    free(pixels);
}

int renderer_init(struct render_data *r, GLuint program) {
    int status = 1;

    r->gl_program = program;
    glUseProgram(r->gl_program);

    // Generate OpenGL objects
    glGenBuffers(1, &r->gl_vbo);
    glGenBuffers(1, &r->gl_ebo);
    glGenVertexArrays(1, &r->gl_vao);
    
    // Load data into VAO
    glBindVertexArray(r->gl_vao);
    glBindBuffer(GL_ARRAY_BUFFER, r->gl_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(SCREEN_QUAD), SCREEN_QUAD, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, r->gl_ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(SCREEN_QUAD_IND), SCREEN_QUAD_IND, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4*sizeof(float), (void*)0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 
                          4*sizeof(float), (void*)(2*sizeof(float)));
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    
    // Unbind the VAO
    glBindVertexArray(0);
    
    // Create the screen texture
    size_t size = (CHUNK_SIZE);
    size_t area = size*size;
    GLubyte *pixels = calloc(area * 4, sizeof(GLubyte));
    if (!pixels) { 
        status = 0;
        goto out;
    }
    memset(pixels, 75, area*4* sizeof(GLubyte));

    // Create and bind texture object
    glGenTextures(1, &r->gl_tex);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, r->gl_tex);
    // Texture parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);	
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    // Load texture data 
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, size, size, 0, GL_RGB, 
                 GL_UNSIGNED_BYTE, pixels);
    
    // Unbind texture
    glBindTexture(GL_TEXTURE_2D, 0);
    free(pixels);

out:
    return status;      
}

void renderer_draw(struct render_data *r){
    glUseProgram(r->gl_program);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, r->gl_tex);

    glBindVertexArray(r->gl_vao);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    
    glad_glBindTexture(GL_TEXTURE_2D, 0);
    glBindVertexArray(0);
}

