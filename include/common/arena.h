#include "common/type.h"

typedef struct Arena__Chunk Arena__Chunk;
typedef struct Arena {
    Arena__Chunk* top;
} Arena;

typedef struct ArenaState {
    Arena__Chunk* top;
    usize used;
} ArenaState;

void arena_init(Arena* arena);
void arena_destroy(Arena* arena);
void* arena_alloc(Arena* arena, usize size, usize align);

ArenaState arena_save(Arena* arena);
void arena_restore(Arena* arena, ArenaState save);
