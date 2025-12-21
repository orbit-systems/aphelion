#include "apollo.h"
#include "common/vec.h"

ApoBuilder* apo_new(ApoObjectKind kind) {
    ApoBuilder* b = malloc(sizeof(*b));
    memset(b, 0, sizeof(*b));

    b->undef_symbols = vec_new(ApoSymbol, 32);
    b->abs_symbols = vec_new(ApoSymbol, 32);
    b->datapool = vec_new(u8, 512);
    b->sections = vec_new(ApoBuilderSection, 16);

    b->datapool.len++;

    return b;
}

ApoHandle apo_intern_string(ApoBuilder* b, const u8* data, u32 data_len) {
    ApoHandle start = b->datapool.len;
    vec_reserve(&b->datapool, data_len + 1);
    vec_append_many(&b->datapool, data, data_len);
    vec_append(&b->datapool, 0);
    return start;
}

// assume align is a power of two
static inline uintptr_t align_forward(uintptr_t ptr, uintptr_t align) {
    return (ptr + align - 1) & ~(align - 1);
}

ApoHandle apo_intern_data(ApoBuilder* b, const u8* data, u32 data_len, usize alignment) {
    usize start = align_forward(b->datapool.len, alignment);
    usize growth = data_len + start - b->datapool.len;

    vec_reserve(&b->datapool, growth);
    b->datapool.len = start;
    vec_append_many(&b->datapool, data, data_len);

    return (ApoHandle) start;
}

#define section_ptr(b, handle) (&(b)->sections.at[handle])
#define symbol_ptr(s, handle) (&(s)->symbols.at[handle])

ApoHandle apo_new_section(ApoBuilder* b, ApoHandle name, ApoSectionFlags flags, u64 map_address) {
    ApoHandle sh = b->sections.len;

    vec_append(&b->sections, (ApoBuilderSection){0});
    ApoBuilderSection* s = section_ptr(b, sh);
    s->name = name;
    s->flags = flags;
    s->map_address = map_address;
    s->symbols = vec_new(ApoSymbol, 16);
    s->relocs = vec_new(ApoRelocation, 16);
    s->content = vec_new(u8, 128);

    return sh;
}

ApoHandle apo_new_symbol(ApoBuilder* b, ApoHandle section, ApoHandle name, ApoSymbolBind bind, u32 offset) {
    ApoBuilderSection* s = section_ptr(b, section);
    
    ApoHandle symh = s->symbols.len;
    vec_append(&s->symbols, (ApoSymbol){0});

    ApoSymbol* sym = symbol_ptr(s, symh);

    sym->name = name;
    sym->offset = offset;
    sym->bind = bind;

    return symh;
}

ApoHandle apo_new_undef_symbol(ApoBuilder* b, ApoHandle name, ApoSymbolBind bind) {
    
    ApoHandle symh = b->undef_symbols.len;
    vec_append(&b->undef_symbols, (ApoSymbol){0});

    ApoSymbol* sym = &b->undef_symbols.at[symh];

    sym->name = name;
    sym->bind = bind;

    return symh;
}
