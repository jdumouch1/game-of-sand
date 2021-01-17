#pragma once
#include <stdio.h>
#include <stdlib.h>
#include "../common/common.h"


#ifdef DEBUG
#define GOS_SHADER_DEBUG
#endif


int shader_compile_file(GLuint *shader, GLenum shader_type, const char *path);

int shader_program_link(GLuint *program, GLuint vertex_shader, 
                        GLuint fragment_shader);

int shader_program_from_files(GLuint *program, const char *vert_path,
                              const char *frag_path);
