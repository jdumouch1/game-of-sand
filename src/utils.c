#include "../include/utils.h"

void linalg_ortho(float *matrix, int left, int right, int bottom, int top){
    memset(matrix, 0, sizeof(float) * 16);

    matrix[0] = 2.0/(right-left);
    matrix[5] = 2.0/(top-bottom);
    matrix[10] =  1.0;
    matrix[12] = -1.0;
    matrix[13] = -1.0;
    matrix[15] =  1.0;
}


void shuffle(int *arr, size_t n) {
    if (n <=1) { return; }
    for (size_t i = 0; i < n; i++){
        size_t rand_elem = i + rand() / (RAND_MAX / (n-i) + 1);
        int tmp = arr[rand_elem];
        arr[rand_elem] = arr[i];
        arr[i] = tmp;
    }
}


void brensenham(vec2 *line, size_t *size, vec2 start, vec2 end){
    *size = 0;

    int dx =  abs (end.x - start.x), sx = start.x < end.x ? 1 : -1;
    int dy = -abs (end.y - start.y), sy = start.y < end.y ? 1 : -1; 
    int err = dx + dy, e2; /* error value e_xy */
 
    for (;;){  /* loop */
        line[(*size)++] = (vec2){.x=start.x, .y=start.y};
        if (start.x == end.x && start.y == end.y) break;
        e2 = 2 * err;
        if (e2 >= dy) { err += dy; start.x += sx; } /* e_xy+e_x > 0 */
        if (e2 <= dx) { err += dx; start.y += sy; } /* e_xy+e_y < 0 */
    }
}
 
