#ifndef APOLLO_H
#define APOLLO_H

#include "common/type.h"

typedef struct ApoHeader ApoHeader;
typedef struct ApoSectionHeader ApoSectionHeader;
typedef struct ApoSymbol ApoSymbol;
typedef u32 ApoStringIndex;
#define APO_NULL_STRING 0
#define APO_NULL_SECTION 0
#define APO_NULL_SYMBOL 0

typedef enum : u8 {
    APO_OBJ_RELOCATABLE,
    APO_OBJ_SHARED,
    APO_OBJ_EXECUTABLE,
} ApoObjectKind;

// typedef enum : u8 {
//     APO_CMODEL_U32, // symbols are in [0, U32_MAX]
//     APO_CMODEL_I32, // symbols are in [I32_MIN, I32_MAX]
//     APO_CMODEL_ANY, // symbols can be any 64-bit value
// } ApoCodeModel;

struct ApoHeader {
    // {0xB2, 'a', 'p', 'o'}
    u8 magic[4];

    ApoObjectKind kind;

    // symbol index to begin execution at, if relevant.
    u32 entry_symbol;

    u32 section_count;
    u32 symbol_count;
    u32 reloc_count;
    u32 strings_size;
    u32 content_size;
};

typedef enum : u16 {
    // should be loaded into program memory.
    APO_SECFL_LOAD = 1 << 0,
    
    // should be loaded with write permissions
    APO_SECFL_WRITE = 1 << 1,
    
    // should be loaded with execute permissions
    APO_SECFL_EXECUTE = 1 << 2,

    // each thread should have a unique copy of this section.
    APO_SECFL_THREADLOCAL = 1 << 3,
    
    // initialized with zeroes, doesn't actually take up data
    APO_SECFL_BLANK = 1 << 4,
    
    // this section is considered "pinned" to its preferred address.
    // emit an error if there are mapping conflicts.
    APO_SECFL_PINNED = 1 << 5,

    // can be replaced by another section with the same name.
    // sections with this flag should not be part of a section group,
    // rather the section group header should have this flag.
    APO_SECFL_COMMON = 1 << 6,
    
    // header of a section group.
    // ".content_size" now means how many sections are in the group.
    // section headers in a section group must be placed
    // continugously after the section header.
    APO_SECFL_GROUP_HEADER = 1 << 7,

    // part of a section group.
    APO_SECFL_GROUPED = 1 << 8,
} ApoSectionFlags;

struct ApoSectionHeader {
    ApoStringIndex name;
    ApoSectionFlags flags;
    u8 alignment;
    // ApoCodeModel cmodel;

    u64 map_address;

    u32 reloc_start;
    u32 reloc_count;

    u32 content_start;
    u32 content_size;
};

typedef enum : u8 {
    // default align 8 bytes
    APO_RELOC_W,

    // default align 4 bytes
    APO_RELOC_H0,
    APO_RELOC_H1,

    // default align 2 bytes
    APO_RELOC_Q0_JL, // cut off bits 0..1 for JL
    APO_RELOC_Q0,
    APO_RELOC_Q1,
    APO_RELOC_Q2,
    APO_RELOC_Q3,

    // default align 4 bytes
    APO_RELOC_CALL,

    // default align 4 bytes
    APO_RELOC_LI,

    // default align 4 bytes
    APO_RELOC_BRANCH,
} ApoRelocationKind;

typedef enum : u8 {
    // when calculating relocation value, subtract the current location.
    // (adding `ip` in code should give the correct value)
    APO_RELOCFL_RELATIVE = 1 << 0,

    // this is used to suppress value truncation errors.
    APO_RELOCFL_NOERROR = 1 << 1,

    // this relocation may not be aligned natively.
    APO_RELOCFL_UNALIGNED = 1 << 2,

    // linkers/loaders may relax/optimize relocations.
    APO_RELOCFL_RELAX = 1 << 3,
} ApoRelocationFlags;

struct ApoRelocation {
    u32 symbol;
    ApoRelocationKind kind;
    ApoRelocationFlags flags;
    i16 addend;
    u32 offset; // offset into section where the relocation must occur
};

typedef enum : u8 {
    APO_SYMBIND_GLOBAL,
    APO_SYMBIND_WEAK,
    APO_SYMBIND_LOCAL,
} ApoSymbolBind;

typedef enum: u8 {
    // the symbol is not defined yet.
    APO_SYMFL_UNDEFINED = 1 << 0,

    // the symbol is an absolute value, not defined relative to anything.
    // use ".offset" as an (8-byte aligned) offset into the data pool
    APO_SYMFL_ABSOLUTE = 1 << 1,

    // allow the linker to suffix the name of a symbol if symbol conflicts occur
    // when combining multiple object files together
    APO_SYMFL_SUFFIX = 1 << 2,
} ApoSymbolFlags;

struct ApoSymbol {
    ApoStringIndex name;
    u32 offset;
    ApoSymbolBind bind;
    ApoSymbolFlags flags;
};

#endif // APOLLO_H
