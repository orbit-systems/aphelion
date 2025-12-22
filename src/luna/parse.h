#ifndef LUNA_PARSE_H
#define LUNA_PARSE_H

#include "common/str.h"
#include "common/strmap.h"
#include "common/util.h"

#include "luna.h"
#include "lex.h"
#include "aphelion.h"

#include "apollo/apollo.h"

typedef struct Section {
    /// Section name.
    /// \note owned by the corresponding LunaInstance, either from source or the permanent arena.
    const char* name;

    /// Length of section name.
    u16 name_len;

    u8 alignment_p2;

    ApoSectionFlags flags;

    u64 map_address;

    union {
        Vec(u8) bytes;
        u32 group_len;
    };
} Section;

typedef enum : u8 {
    SYM_LOCAL = 0,
    SYM_GLOBAL,
    SYM_WEAK,
    SYM_UNDEFINED,
} SymbolBind;

typedef struct Symbol {
    const char* name;
    u16 name_len;
} Symbol;

typedef struct SubLabel {
    const char* name;
    u16 name_len;
    u32 parent_label;
} SubLabel;

typedef struct Parser {
    LunaInstance* luna;

    Vec(Token) tokens;
    usize cursor;
    Token current;

    // everything about the assembly file being created

    Vec(Section*) sections;

    Section* current_section_group;
    Section* current_section;

    u64 address;

    StrMap symbol_indexes;
    Vec(Symbol) symbols;

    // Vec(SubLabel) sub_labels;
} Parser;

Parser parser_new(LunaInstance* luna, Vec(Token) tokens);
void parse_file(Parser* p);

#endif // LUNA_PARSE_H
