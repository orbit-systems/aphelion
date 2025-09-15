#pragma once

// #include "orbit.h"
#include "common/str.h"
#include "common/type.h"

// strmap associates a string value with a void*.

typedef struct StrMap {
    string* keys;
    void** vals;
    u32 cap; // capacity
    u32 size; // number of items
} StrMap;

#define STRMAP_NOT_FOUND ((void*)0xDEADBEEF)

void strmap_init(StrMap* sm, u32 capacity);
void strmap_reset(StrMap* sm);
void strmap_destroy(StrMap* sm);
void strmap_put(StrMap* sm, string key, void* val);
void strmap_remove(StrMap* sm, string key);
void* strmap_get(StrMap* sm, string key);
