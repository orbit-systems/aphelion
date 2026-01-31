#ifndef APOLLO_H
#define APOLLO_H

#include "common/type.h"

typedef struct ApoHeader ApoHeader;
typedef struct ApoSectionHeader ApoSectionHeader;
typedef struct ApoSymbol ApoSymbol;
typedef struct ApoRelocation ApoRelocation;

#define APO_NULL_SYMBOL_INDEX 0

/// Kinds of object file.
typedef enum : u8 {
    /// Static object code.
    APO_OBJ_STATIC = 0,
    /// A shared library.
    APO_OBJ_SHARED = 1,
    /// An executable program.
    APO_OBJ_EXECUTABLE = 2,
} ApoObjectKind;

struct ApoHeader {
    /// Four bytes at the beginning of the object file,
    /// {0xB2, 'a', 'p', 'o'}
    u8 magic[4];

    /// Kind of object file described.
    ApoObjectKind kind;

    /// Symbol index to begin execution at, if applicable.
    u32 entry_symbol;

    /// Number of sections in the section table.
    u32 section_count;

    /// Number of symbols in the symbol table.
    u32 symbol_count;

    /// Number of relocations in the relocation table.
    u32 reloc_count;

    /// Size of the section contents pool.
    u32 content_size;

    /// Size of the data pool.
    u32 data_pool_size;
};

typedef enum : u16 {
    /// This section should not be loaded into program memory.
    APO_SECFL_UNMAPPED = 1 << 0,

    /// This section should be loaded with write permissions.
    APO_SECFL_WRITABLE = 1 << 1,

    /// This section should be loaded with execute permissions.
    APO_SECFL_EXECUTABLE = 1 << 2,

    /// Each thread should have a unique copy of this section.
    APO_SECFL_THREADLOCAL = 1 << 3,

    /// This section is initialized with zeroes and doesn't actually take up data in the file.
    APO_SECFL_BLANK = 1 << 4,

    /// This section is considered "pinned" to its `.map_address`.
    /// Emit an error if there are mapping conflicts.
    APO_SECFL_PINNED = 1 << 5,

    /// This section may be removed in a "final link" if no symbol defined in it is referenced.
    APO_SECFL_NONVOLATILE = 1 << 6,

    /// This section can be replaced by another section with the same name.
    APO_SECFL_COMMON = 1 << 7,

    /// This section shall not be concatenated with sections with the same name and same section flags.
    APO_SECFL_UNIQUE = 1 << 8,
} ApoSectionFlags;

struct ApoSectionHeader {
    u32 name;
    ApoSectionFlags flags;
    u8 alignment_p2;

    u64 map_address;

    u32 reloc_start;
    u32 reloc_count;

    u32 content_start;
    u32 content_size;

    /// if non-zero, this section should only be included
    /// if its parent is included.
    u32 parent;
};

typedef enum : u8 {
    /// Insert an absolute symbol word. (width 8, align 8)
    APO_RELOC_WORD,

    /// Insert an absolute symbol word. (width 8, align 1)
    APO_RELOC_WORD_UNALIGNED,

    /// Insert a relative address for a 32-bit relative call. (width 8, align 4)
    APO_RELOC_RCALL,

    /// Insert an absolute address for a 64-bit absolute call. (width 16, align 4)
    APO_RELOC_FCALL,

    // Insert an absolute address for loading into a register. (width 16, align 4)
    APO_RELOC_LI,
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

struct ApoSymbol {
    // index of the symbol name
    u32 name;
    u32 offset;
    ApoSymbolBind bind;
};
#endif // APOLLO_H
