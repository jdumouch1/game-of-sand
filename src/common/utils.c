#include "../../include/common/utils.h"

// mat4 layout:
// { 0 1 2 3
//   4 5 6 7
//   8 9 A B
//   C D E F }

mat4 lalg_cross(mat4 *m1, mat4 *m2){
    mat4 m;
    memcpy(&m, m1, sizeof(mat4));

    for (int r = 0; r < 4; ++r){
        int row_id = r * 4;
        for (int c = 0; c < 4; ++c){
            m.m[row_id + c] = 
                (m1->m[row_id + 0] * m2->m[c + 0x0]) +
                (m1->m[row_id + 1] * m2->m[c + 0x4]) +
                (m1->m[row_id + 2] * m2->m[c + 0x8]) + 
                (m1->m[row_id + 3] * m2->m[c + 0xC]);
        }
    }
    return m;
}

mat4 lalg_ortho(int left, int right, int bottom, int top){
    mat4 matrix;

    memset(matrix.m, 0, sizeof(matrix.m));

    matrix.m[0x0] = 2.0/(right-left);
    matrix.m[0x5] = 2.0/(top-bottom);
    matrix.m[0xA] =  1.0;

    matrix.m[0x3] = -1.0;
    matrix.m[0x7] = -1.0;
    matrix.m[0xF] =  1.0;

    return matrix;
}

mat4 lalg_translation(float x, float y, float z){
    mat4 matrix = IDENTITY_MATRIX;
    matrix.m[0x3] = x;
    matrix.m[0x7] = y;
    matrix.m[0xB] = z;

    return matrix;
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
 
