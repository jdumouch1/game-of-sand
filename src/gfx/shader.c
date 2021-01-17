#include "../../include/gfx/shader.h"

int shader_program_from_files(GLuint *program, const char *vert_path, 
                              const char *frag_path) {
    GLuint vert_shader, frag_shader;
    if (!shader_compile_file(&vert_shader, GL_VERTEX_SHADER, vert_path)){
        goto err;
    }
    if (!shader_compile_file(&frag_shader, GL_FRAGMENT_SHADER, frag_path)){
        goto err;
    }
    if (!shader_program_link(program, vert_shader, frag_shader)){
        goto err;
    }

    glDeleteShader(vert_shader);
    glDeleteShader(frag_shader);

    return 1;
err:
    return 0;
}

int shader_program_link(GLuint *program, GLuint vertex_shader, 
                        GLuint fragment_shader){
    
    // Link shaders to program
    *program = glCreateProgram();
    glAttachShader(*program, vertex_shader);
    glAttachShader(*program, fragment_shader);
    glLinkProgram(*program);
    
    // Check link status
    GLint link_success; 
    GLchar link_log[512]; 
    glGetProgramiv(*program, GL_LINK_STATUS, &link_success);
    if (!link_success) {
        glGetProgramInfoLog(*program, 512, NULL, link_log);
        fprintf(stderr, "Failed to link shader program:\n%s\n", link_log);
        goto err;
    }

    glDetachShader(*program, vertex_shader);
    glDetachShader(*program, fragment_shader);
    
    return 1;
err:
    return 0;
}

int shader_compile_file(GLuint *shader, GLenum shader_type, const char *path){
    #ifdef GOS_SHADER_DEBUG
        printf("Compiling \"%s\" as %s\n", path, 
                shader_type == GL_VERTEX_SHADER ? "VERTEX_SHADER" 
                                                : "FRAGMENT_SHADER");
    #endif
    
    // Open file
    FILE *file = fopen(path, "r");
    if (!file){
        fprintf(stderr, "Failed to open shader file \"%s\"\n", path);
        goto err;
    }
    
    // Get file size
    fseek(file, 0, SEEK_END);
    int64_t num_bytes = ftell(file);
    if (!num_bytes){
        fprintf(stderr, "Shader file \"%s\" is empty.\n", path);
        goto err_after_file;
    }
    fseek(file, 0, SEEK_SET);

    // Allocate buffer
    char *buffer = calloc(num_bytes, sizeof(char));
    if (!buffer){
        fprintf(stderr, "Failed to allocate buffer space "
                        "for shader file \"%s\"\n", path);
        goto err_after_file;
    }
    
    // Read file into buffer
    fread(buffer, sizeof(char), num_bytes, file);
    
    #ifdef GOS_SHADER_DEBUG
        printf("------------\n");
        printf("Read %ld bytes:\n", num_bytes);
        printf("------------\n%s\n", buffer);
        printf("------------\n");
    #endif

    *shader = glCreateShader(shader_type);
    const char *src = (const char *)buffer;
    glShaderSource(*shader, 1, &src, NULL);
    glCompileShader(*shader);
    
    // Memory cleanup
    fclose(file);
    free(buffer);
    
    // Check compilation status
    GLint compile_success; 
    GLchar compile_log[512]; 
    glGetShaderiv(*shader, GL_COMPILE_STATUS, &compile_success);
    if (!compile_success) {
        glad_glGetShaderInfoLog(*shader, 512, NULL, compile_log);
        fprintf(stderr, "Failed to compile shader \"%s\":\n%s\n", 
                path, compile_log);
        goto err;
    }
    // Return success
    return 1;

err_after_file:
    fclose(file);
err:
    return 0;
}
