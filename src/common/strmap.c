#include <string.h>

#include "common/util.h"
#include "common/strmap.h"

#define MAX_SEARCH 30
#define TOMBSTONE (void*)0xDEADBEEFDEADBEEF

static u64 FNV_1a(string key) {
    const u64 FNV_OFFSET = 14695981039346656037ull;
    const u64 FNV_PRIME = 1099511628211ull;

    u64 hash = FNV_OFFSET;
    for_n(i, 0, key.len) {
        hash ^= (u64)(u8)(key.raw[i]);
        hash *= FNV_PRIME;
    }
    return hash;
}

void strmap_init(StrMap* hm, u32 capacity) {
    hm->size = 0;
    hm->cap = capacity;
    hm->vals = malloc(sizeof(hm->vals[0]) * hm->cap);
    hm->keys = malloc(sizeof(hm->keys[0]) * hm->cap);
    memset(hm->vals, 0, sizeof(hm->vals[0]) * hm->cap);
    memset(hm->keys, 0, sizeof(hm->keys[0]) * hm->cap);
}

void strmap_destroy(StrMap* hm) {
    if (hm->keys) free(hm->keys);
    if (hm->vals) free(hm->vals);
    *hm = (StrMap){0};
}

void strmap_put(StrMap* hm, string key, void* val) {
    if (hm == nullptr) return;
    if (is_null_str(key)) return;
    size_t hash = FNV_1a(key);


    // search for nearby free slot
    for_n(index, 0, min(MAX_SEARCH, hm->cap)) {
        size_t i = (index + hash) % hm->cap;
        if (hm->keys[i].raw == TOMBSTONE || hm->keys[i].raw == nullptr) {
            ++hm->size;
            hm->keys[i] = key;
            hm->vals[i] = val;
            return;
        } else if (string_eq(hm->keys[i], key)) {
            hm->keys[i] = key;
            hm->vals[i] = val;
            return;
        }
    }

    // we gotta resize
    StrMap new_hm;
    strmap_init(&new_hm, hm->cap * 2);

    // copy all the old entries into the new hashmap
    for (size_t i = 0; i < hm->cap; i++) {
        if (hm->keys[i].raw == nullptr) continue;
        strmap_put(&new_hm, hm->keys[i], hm->vals[i]);
    }
    strmap_put(&new_hm, key, val);

    // destroy old map
    free(hm->keys);
    free(hm->vals);
    *hm = new_hm;
}

void* strmap_get(StrMap* hm, string key) {
    if (!key.raw) return STRMAP_NOT_FOUND;
    size_t hash_index = FNV_1a(key) % hm->cap;

    // key found in first slot
    if (string_eq(hm->keys[hash_index], key)) {
        return hm->vals[hash_index];
    }

    // linear search next slots
    for_n(index, 1, min(MAX_SEARCH, hm->cap)) {
        size_t i = (index + hash_index) % hm->cap;
        if (hm->keys[i].raw == nullptr) return STRMAP_NOT_FOUND;
        if (string_eq(hm->keys[i], key)) return hm->vals[i];
    }

    return STRMAP_NOT_FOUND;
}

void strmap_remove(StrMap* hm, string key) {
    if (!key.raw) return;
    size_t key_hash = FNV_1a(key) % hm->cap;

    // linear search next slots
    for_n(index, 0, min(MAX_SEARCH, hm->cap)) {
        size_t i = (index + key_hash) % hm->cap;
        if (hm->keys[i].raw == nullptr) return;
        if (string_eq(hm->keys[i], key)) {
            hm->keys[i].len = 0;
            hm->keys[i].raw = TOMBSTONE;
            hm->vals[i] = nullptr;
            --hm->size;
            return;
        }
    }
}

void strmap_reset(StrMap* hm) {
    memset(hm->vals, 0, sizeof(hm->vals[0]) * hm->cap);
    memset(hm->keys, 0, sizeof(hm->keys[0]) * hm->cap);
    hm->size = 0;
}
