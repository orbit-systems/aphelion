#include <string.h>
#include <stdio.h>

#include "common/vec.h"

#define WEAK __attribute((weak))

WEAK Vec(void) _vec_new(size_t stride, size_t initial_cap) {
    if (initial_cap < 2) {
        initial_cap = 2;
    }
    Vec(void) vec = (char*)malloc(sizeof(VecHeader) + stride * initial_cap) + sizeof(VecHeader);
    vec_cap(vec) = initial_cap;
    vec_len(vec) = 0;

    return vec;
}

WEAK void _vec_reserve(Vec(void)* v, size_t stride, size_t slots) {
    if (slots + vec_len(*v) > vec_cap(*v)) {
        vec_cap(*v) = slots + (vec_cap(*v) >> 1);
        *v = realloc(vec_header(*v), sizeof(VecHeader) + vec_cap(*v) * stride);
    }
}

WEAK void _vec_reserve1(Vec(void)* v, size_t stride) {
    if (vec_len(*v) + 1 > vec_cap(*v)) {
        vec_cap(*v) = (vec_cap(*v) * 3) / 2;
        *v = realloc(vec_header(*v), sizeof(VecHeader) + vec_cap(*v) * stride);
    }
}

WEAK void _vec_shrink(Vec(void)* v, size_t stride) {
    
}

WEAK void _vec_destroy(Vec(void)* v) {
    free(vec_header(*v));
    *v = nullptr;
}

// WEAK void _vec_shrink(_VecGeneric* v, size_t stride) {
//     if (v->len == v->cap) {
//         return;
//     }
//     v->at = realloc(v->at, v->len * stride);
//     v->cap = v->len;
// }
