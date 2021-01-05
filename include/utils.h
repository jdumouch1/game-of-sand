#pragma once
#include <stdlib.h>
#include <memory.h>

#include "common.h"

void linalg_ortho(float *matrix, int left, int right, int bottom, int top);

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
