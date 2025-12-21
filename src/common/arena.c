#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "common/util.h"
#include "common/arena.h"

#define ARENA_CHUNK_DATA_SIZE 32768
typedef struct Arena__Chunk {
    Arena__Chunk* prev;
    Arena__Chunk* next;
    usize used;
    u8 data[ARENA_CHUNK_DATA_SIZE];
} Arena__Chunk;

// assume align is a power of two
static inline uintptr_t align_forward(uintptr_t ptr, uintptr_t align) {
    return (ptr + align - 1) & ~(align - 1);
}

// return nullptr if cannot allocate
static void* chunk_alloc(Arena__Chunk* ch, usize size, usize align) {
    usize new_used = align_forward(ch->used, align);
    if (new_used + size > ARENA_CHUNK_DATA_SIZE) {
        return nullptr;
    }
    ch->used = new_used;
    void* mem = &ch->data[ch->used];
    ch->used += size;
    return mem;
}

void arena_init(Arena* arena) {
    arena->top = malloc(sizeof(*arena->top));
    arena->top->next = nullptr;
    arena->top->prev = nullptr;
    arena->top->used = 0;
}

void arena_destroy(Arena* arena) {
    Arena__Chunk* top = arena->top;
    // traverse up to top of the list
    while (top->next) {
        top = top->next;
    }
    // destroy any saved blocks below
    for (Arena__Chunk* ch = top, *prev = ch->prev; ch != nullptr; ch = prev, prev = prev->prev) {
        free(ch);
    }
    arena->top = nullptr;
}

void* arena_alloc(Arena* arena, usize size, usize align) {
    void* mem = chunk_alloc(arena->top, size, align);
    if (mem) {
        return mem;
    }

    Arena__Chunk* new_chunk = arena->top->next;
    if (new_chunk == NULL) {
        new_chunk = malloc(sizeof(*new_chunk));
        new_chunk->prev = arena->top;
        new_chunk->next = nullptr;
        arena->top->next = new_chunk;
    }
    new_chunk->used = 0;
    arena->top = new_chunk;
    mem = chunk_alloc(new_chunk, size, align);
    if (mem) {
        return mem;
    }
    CRASH("unable to arena alloc size %zu align %zu", size, align);
}

ArenaState arena_save(Arena* arena) {
    return (ArenaState){
        .top = arena->top,
        .used = arena->top->used,
    };
}

void arena_restore(Arena* arena, ArenaState save) {
    arena->top = save.top;
    arena->top->used = save.used;
}

string arena_strcat(Arena* arena, string a, string b, bool nul) {
    usize len = a.len + b.len;
    char* str = arena_alloc(arena, len + (usize)nul, 1);

    memcpy(str, a.raw, a.len);
    memcpy(str + a.len, b.raw, b.len);
    if (nul) {
        str[len] = 0;
    }

    return string_make(str, len);
}

string arena_strcat3(Arena* arena, string a, string b, string c, bool nul) {
    usize len = a.len + b.len + c.len;
    char* str = arena_alloc(arena, len + (usize)nul, 1);

    memcpy(str, a.raw, a.len);
    memcpy(str + a.len, b.raw, b.len);
    memcpy(str + a.len + b.len, c.raw, c.len);
    if (nul) {
        str[len] = 0;
    }

    return string_make(str, len);
}

const char* arena_cstring(Arena* arena, string a) {
    char* str = arena_alloc(arena, a.len + 1, 1);
    memcpy(str, a.raw, a.len);
    str[a.len] = 0;

    return str;
}

string arena_strdup(Arena* arena, string a) {
    char* str = arena_alloc(arena, a.len, 1);
    memcpy(str, a.raw, a.len);

    return string_make(str, a.len);
}

const char* arena_cstrdup(Arena* arena, const char* s) {
    usize len = strlen(s);
    char* str = arena_alloc(arena, len + 1, 1);
    strcpy(str, s);
    
    return s;
}
