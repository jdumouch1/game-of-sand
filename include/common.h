#pragma once
#include <stddef.h>
#include <stdint.h>


#define CHUNK_SCALE 8
#define POINT_SCALE 0

#define CHUNK_SIZE (1<<CHUNK_SCALE)
#define CHUNK_AREA (CHUNK_SIZE * CHUNK_SIZE)
#define POINT_SIZE (1<<POINT_SCALE)

#define SIM_SPEED 0.5

#define TGL_FLAG(n, f) ((n) ^= (f))
#define SET_FLAG(n, f) ((n) |= (f))
#define CLR_FLAG(n, f) ((n) &= ~(f))
#define CHK_FLAG(n, f) ((n) & (f))

#define min(a,b) __extension__ \
    ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
       _a < _b ? _a : _b; })

#define max(a,b) __extension__ \
    ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
       _a > _b ? _a : _b; })

/*
static const size_t CHUNK_SCALE = 9;
static const size_t POINT_SCALE = 1;

static const size_t CHUNK_SIZE = (1<<CHUNK_SCALE);
static const size_t CHUNK_AREA = (CHUNK_SIZE * CHUNK_SIZE);
static const size_t POINT_SIZE = (1<<POINT_SCALE);
*/
struct vec2 {
    int16_t x;
    int16_t y;
};

struct uvec2 {
    uint16_t x;
    uint16_t y;
};


