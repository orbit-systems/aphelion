#ifndef APOLLO_H
#define APOLLO_H

#include "common/type.h"

typedef struct ApoHeader ApoHeader;
typedef struct {u32 _;} ApoStringIndex;

typedef enum : u8 {
    APO_OBJ_RELOCATABLE,
    APO_OBJ_SHARED,
    APO_OBJ_EXECUTABLE,
} ApoObjectKind;

struct ApoHeader {
    // {0xB2, 'a', 'p', 'o'}
    u8 magic[4];
    // symbol index to begin execution at, if relevant.
    u64 exec_symbol; 
};

typedef enum : u32 {
    // executable
    APO_SECTION_X = 1 << 0,
    // writeable
    APO_SECTION_W = 1 << 1,
    // like "bss"
    APO_SECTION_BLANK = 1 << 2,
} ApoSectionFlags;

#endif // APOLLO_H
