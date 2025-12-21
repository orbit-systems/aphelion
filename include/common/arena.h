#include "common/type.h"
#include "common/str.h"

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

string arena_strcat(Arena* arena, string a, string b, bool nul);
string arena_strcat3(Arena* arena, string a, string b, string c, bool nul);
const char* arena_cstring(Arena* arena, string a);
string arena_strdup(Arena* arena, string a);
const char* arena_cstrdup(Arena* arena, const char* s);
