#include "../include/utils.h"

void ortho(float *matrix, int left, int right, int bottom, int top){
    memset(matrix, 0, sizeof(float) * 16);

    matrix[0] = 2.0/(right-left);
    matrix[5] = 2.0/(top-bottom);
    matrix[10] =  1.0;
    matrix[12] = -1.0;
    matrix[13] = -1.0;
    matrix[15] =  1.0;
}


void shuffle(size_t *arr, size_t n) {
    if (n <=1) { return; }
    for (size_t i = 0; i < n; i++){
        size_t rand_elem = i + rand() / (RAND_MAX / (n-i) + 1);
        size_t tmp = arr[rand_elem];
        arr[rand_elem] = arr[i];
        arr[i] = tmp;
    }
}
 
