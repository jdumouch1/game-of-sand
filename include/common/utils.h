#pragma once
#include <stdlib.h>
#include <memory.h>

#include "common.h"

typedef struct {
    float m[16];
} mat4;

static const mat4 IDENTITY_MATRIX = {
    .m = {
        1.0, 0.0, 0.0, 0.0,
        0.0, 1.0, 0.0, 0.0,
        0.0, 0.0, 1.0, 0.0,
        0.0, 0.0, 0.0, 1.0,
    },
};
mat4 lalg_cross(mat4 *m1, mat4 *m2);
mat4 lalg_ortho(int left, int right, int bottom, int top);
mat4 lalg_translation(float x, float y, float z);
void shuffle(int *arr, size_t n);

/*
 * brensenham() - Create a line from one point to another.
 * @ vec2 *line - A buffer of vec2 to store the line.
 * @ size_t *size - A pointer to a size counter. Can be null.
 * @ vec2 start - The point to start the line. (inclusive)  
 * @ vec2 end - The point to end the line. (inclusive)  
 * 
 * This is an implementation of Bresenham's line algorithm,
 * a line is generating using no expensive operations.
 *
 * This implementation was found at:
 *  https://gist.github.com/bert/1085538
 * Incredible work by him, easily the most concise version I've seen.
*/
void brensenham(vec2 *path, size_t *size, vec2 start, vec2 end);

int32_t fast_abs(int32_t a);
