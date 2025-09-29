#ifndef APOLLO_H
#define APOLLO_H

#include "common/str.h"
#include "common/type.h"
#include "common/vec.h"

typedef struct ApoHeader ApoHeader;
typedef struct ApoSectionHeader ApoSectionHeader;
typedef struct ApoSymbol ApoSymbol;
typedef struct ApoRelocation ApoRelocation;
typedef struct ApoBuilder ApoBuilder;
typedef struct ApoBuilderSection ApoBuilderSection;
typedef u32 ApoHandle;

#define APO_NULL_HANDLE 0

typedef enum : u8 {
    APO_OBJ_RELOCATABLE,
    APO_OBJ_SHARED,
    APO_OBJ_EXECUTABLE,
} ApoObjectKind;

struct ApoHeader {
    // {0xB2, 'a', 'p', 'o'}
    u8 magic[4];

    ApoObjectKind kind;

    // symbol index to begin execution at, if relevant.
    u32 entry_symbol;

    u32 section_count;
    u32 symbol_count;
    u32 reloc_count;
    u32 datapool_size;
    u32 content_size;
};

typedef enum : u16 {
    // should be loaded into program memory.
    APO_SECFL_LOAD = 1 << 0,
    
    // should be loaded with write permissions.
    APO_SECFL_WRITABLE = 1 << 1,
    
    // should be loaded with execute permissions.
    APO_SECFL_EXECUTABLE = 1 << 2,

    // each thread should have a unique copy of this section.
    APO_SECFL_THREADLOCAL = 1 << 3,
    
    // initialized with zeroes, doesn't actually take up data.
    APO_SECFL_BLANK = 1 << 4,
    
    // this section is considered "pinned" to its preferred address.
    // emit an error if there are mapping conflicts.
    APO_SECFL_PINNED = 1 << 5,

    // can be replaced by another section with the same name.
    // sections with this flag should not be part of a section group,
    // rather the section group header should have this flag.
    APO_SECFL_COMMON = 1 << 6,

    // this section cannot be removed in a "final link," even if none of its
    // symbols are referenced.
    APO_SECFL_VOLATILE = 1 << 7,

    // sections with the same name and same section flags should be concatenated.
    APO_SECFL_CONCATENATE = 1 << 8,
    
    // header of a section group.
    // ".content_size" now means how many sections are in the group.
    // section headers in a section group must be placed
    // continugously after the section header.
    APO_SECFL_GROUP_HEADER = 1 << 9,

    // part of a section group.
    APO_SECFL_GROUP_MEMBER = 1 << 10,
} ApoSectionFlags;

struct ApoSectionHeader {
    ApoHandle name;
    ApoSectionFlags flags;
    u8 alignment_p2;

    u64 map_address;

    u32 reloc_start;
    u32 reloc_count;

    u32 content_start;
    u32 content_size;
};

typedef enum : u8 {
    // unaligned
    APO_RELOC_W_UNALIGNED,
    // align 8 bytes
    APO_RELOC_W,

    // align 4 bytes
    APO_RELOC_CALL,

    // align 4 bytes
    APO_RELOC_FARCALL,

    // align 4 bytes
    APO_RELOC_LI,

    // align 4 bytes
    APO_RELOC_BRANCH,
} ApoRelocationKind;

struct ApoRelocation {
    u32 symbol;
    ApoRelocationKind kind;
    i16 addend;
    u32 offset; // offset into section where the relocation must occur
};

typedef enum : u8 {
    APO_SYMBIND_GLOBAL,
    APO_SYMBIND_WEAK,
    APO_SYMBIND_LOCAL,
} ApoSymbolBind;

typedef enum: u8 {
    // allow the linker to suffix the name of a symbol if name conflicts occur
    // when combining multiple object files together
    APO_SYMFL_SUFFIX = 1 << 0,

    // this symbol cannot be removed in a "final link,"
    // even if it is not referenced.
    APO_SYMFL_VOLATILE = 1 << 1,

} ApoSymbolFlags;

struct ApoSymbol {
    ApoHandle name;
    u32 offset;
    ApoSymbolBind bind;
    ApoSymbolFlags flags;
};

Vec_typedef(ApoBuilderSection);
Vec_typedef(ApoSymbol);
Vec_typedef(ApoRelocation);
Vec_typedef(u8);

/* reader/writer/builder API */

struct ApoBuilder {
    Vec(ApoBuilderSection) sections;
    Vec(u8) datapool;

    Vec(ApoSymbol) undef_symbols;
    Vec(ApoSymbol) abs_symbols;
};

struct ApoBuilderSection {
    ApoHandle name;
    ApoSectionFlags flags;
    u64 map_address;

    Vec(ApoSymbol) symbols;
    Vec(ApoRelocation) relocs;
    Vec(u8) content;
};

/// create an ApoBuilder based on the apollo file in 'bytes'.
// 
ApoBuilder* apo_read(const u8* bytes, usize len);

// create an ApoBuilder based on the apollo file in 'bytes'.
// operates under the assumption that it will never be modified. 
// operations that modify the builder created with this likely 
// cause fatal crashes. 
// NOTE: relies on 'bytes' as backing memory. 'bytes' should stay valid.
ApoBuilder* apo_readonly(const u8* bytes, usize len);

// create a blank ApoBuilder.
ApoBuilder* apo_new(ApoObjectKind kind);

// export an ApoBuilder to a flat apollo file.
void apo_write(ApoBuilder* b, Vec(u8)* out);

// destroy an ApoBuilder and free the memory associated with it.
void apo_destroy(ApoBuilder* b);

// destroy an ApoBuilder created with apo_readonly().
void apo_destroy_readonly(ApoBuilder* b);

#endif // APOLLO_H
