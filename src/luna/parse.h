#ifndef LUNA_PARSE_H
#define LUNA_PARSE_H

#include "common/strmap.h"

#include "luna.h"
#include "lex.h"
#include "aphelion.h"

#include "apollo/apollo.h"
#include <strings.h>

typedef enum : u8 {
    ELEM_INST_INVALID = 0,
    ELEM_INST__BEGIN = 1,
    // import InstName here

    ELEM_INST__END = INST__COUNT - 1,

    ELEM_ALIGN,
    ELEM_ZERO,
    ELEM_BYTE,
    ELEM_QWORD,
    ELEM_HWORD,
    // next element contains the word value.
    ELEM_WORD,
    // next element contains the pointer to the string's contents
    ELEM_STRING,

    // Label definition.
    ELEM_LABEL,

    // Sublabel (.x:) definition.
    // ELEM_SUBLABEL,

    /// An expression to evaluate after the first pass
    /// and then use as an argument to element preceding it.
    ELEM_EXPR,

    /// Doesn't do anything.
    ELEM_SKIP,
} SectionElemKind;

typedef enum : u8 {
    RELOC_WORD,
    RELOC_WORD_UNALIGNED,
    RELOC_CALL,
    RELOC_FARCALL,
    RELOC_LI,

    RELOC_BRANCH,
} RelocKind;

typedef union SectionElement {
    SectionElemKind kind;
    struct {
        InstName name;
        AphelGpr r1;
        AphelGpr r2;
        AphelGpr r3;
        i32 imm;
    } inst;

    struct {
        SectionElemKind _kind;
        u8 alignment_p2;
    } align;

    struct {
        SectionElemKind _kind;
        u32 len;
    } zero;

    struct {
        SectionElemKind _kind;
        u8 value;
    } byte;

    struct {
        SectionElemKind _kind;
        u16 value;
    } qword;

    struct {
        SectionElemKind _kind;
        u32 value;
    } hword;

    u64 supp_word;

    struct {
        SectionElemKind _kind;
        u32 length;
    } string;
    char* supp_string;

    struct {
        SectionElemKind _kind;
        u32 symbol_index;
    } label;

    struct {
        SectionElemKind _kind;
        u32 index;
    } expr;
} SectionElement;

typedef struct Section {
    /// Section name.
    /// \note owned by the corresponding LunaInstance, either from source or the permanent arena.
    const char* name;

    /// Length of section name.
    u16 name_len;

    u8 alignment_p2;

    ApoSectionFlags flags;

    bool address_specified;
    u64 address;
    u32 bytesize;

    Vec(SectionElement) elements;
    Vec(u32) elem_tokens; // this information is stored separately since it is only used in error reporting.
} Section;

typedef enum : u8 {
    SYM_UNDEFINED = 0,
    SYM_LOCAL,
    SYM_GLOBAL,
    SYM_WEAK,
} SymbolBind;

typedef struct Symbol {
    const char* name;
    u16 name_len;
    SymbolBind bind;
    u32 section_def;
    u32 section_offset;
} Symbol;

typedef enum : u8 {
    CEXPR_VALUE,
    CEXPR_SYMBOL_REF,

    CEXPR_NEG,
    CEXPR_NOT,

    CEXPR_ADD,
    CEXPR_SUB,
    CEXPR_MUL,
    CEXPR_DIV,
    CEXPR_REM,

    CEXPR_AND,
    CEXPR_OR,
} ComplexExprKind;

typedef struct ComplexExpr {
    ComplexExprKind kind;
    u32 token_index;
    union {
        u64 value;
        u32 symbol_ref;
        u32 un;
        struct {
            u32 lhs;
            u32 rhs;
        } bin;
    };
} ComplexExpr;

typedef struct Parser {
    LunaInstance* luna;

    Vec(Token) tokens;
    usize cursor;
    Token current;

    // everything about the assembly file being created

    Vec(Section*) sections;

    Section* current_section;
    u32 current_section_index;

    Vec(ComplexExpr) exprs;

    StrMap symbol_indexes;
    Vec(Symbol) symbols;

    // Vec(SubLabel) sub_labels;
} Parser;

typedef struct Object {
    LunaInstance* luna;
    Vec(Token) tokens;
    Vec(Section*) sections;
    Vec(ComplexExpr) exprs;
    StrMap symbol_indexes;
    Vec(Symbol) symbols;
} Object;

#define ORIGIN_CONSTANT 0xFFFF'FFFF'FFFF'FFFF

typedef struct ExprValue {
    u64 value;
    u64 origin;
} ExprValue;

#define EXPR_CONST(v) (ExprValue){v, ORIGIN_CONSTANT}
#define EXPR_W_ORIGIN(v, o) (ExprValue){v, o}

ExprValue evaluate_expr(const Object* restrict o, u32 expr_index);

Parser parser_new(LunaInstance* luna, Vec(Token) tokens);
Object parse_tokenbuf(Parser* p);

void object_dbgprint(const Object* o);
void object_trace(Object* o);

typedef struct TokenSpan {
    usize start;
    usize end;
} TokenSpan;

#endif // LUNA_PARSE_H
