#include <assert.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdckdint.h>


#include "common/portability.h"
#include "common/strmap.h"
#include "common/util.h"
#include "common/vec.h"

#include "parse.h"
#include "aphelion.h"
#include "apollo/apollo.h"
#include "lex.h"
#include "luna.h"
#include "reporting.h"

static SourceFileId find_source_from_ptr(Vec(SourceFile) files, const char* s) {
    for_n(i, 0, vec_len(files)) {
        string source = files[i].source;
        if (source.raw <= s && s <= source.raw + source.len) {
            return (SourceFileId){i};
        }
    }
    CRASH("cannot find corresponding source file");
}

#define parse_warn(p, token_index, msg, ...) \
    parse_error__internal(p, REPORT_WARNING, token_start(p->tokens[token_index]), strprintf(msg __VA_OPT__(,) __VA_ARGS__))

#define parse_warn_at_ptr(p, ptr, msg, ...) \
    parse_error__internal(p, REPORT_WARNING, ptr, strprintf(msg __VA_OPT__(,) __VA_ARGS__))

#define parse_error(p, token_index, msg, ...) \
    parse_error__internal(p, REPORT_ERROR, token_start(p->tokens[token_index]), strprintf(msg __VA_OPT__(,) __VA_ARGS__))

#define parse_error_at_ptr(p, ptr, msg, ...) \
    parse_error__internal(p, REPORT_ERROR, ptr, strprintf(msg __VA_OPT__(,) __VA_ARGS__))

static void parse_error__internal(Parser* p, ReportKind severity, const char* begin, string msg) {

    SourceFileId fileid = find_source_from_ptr(
        p->luna->files,
        begin
    );
    SourceFile file = p->luna->files[fileid._];

    Report* r = report_new(severity, msg, &p->luna->files);

    usize span_start = begin - file.source.raw;
    usize span_end = span_start + 1;

    report_add_label(r, REPORT_LABEL_PRIMARY, fileid, span_start, span_end, strlit(""));

    report_render(stderr, r);
    report_destroy(r);

    if (severity == REPORT_ERROR) {
        exit(1);
    }
}

static void parse_instruction(Parser* p);

const bool inst_name_imm_signed[INST__COUNT] = {
    [INST_SSI_C] = true,
    [INST_MUL] = true,
    [INST_IMULH] = true,
    [INST_IDIV] = true,
    [INST_MULI] = true,
    [INST_IREM] = true,
    [INST_IDIVI] = true,
    [INST_IREMI] = true,
    [INST_SEQ] = true,
    [INST_SILT] = true,
    [INST_SILE] = true,
    [INST_SEQI] = true,
    [INST_SILTI] = true,
    [INST_SILEI] = true,
    [INST_BZ] = true,
    [INST_BN] = true,
};

static u8 inst_to_imm[INST__COUNT] = {
    /* CINVAL                 m  i d */
    [INST_CINVAL_BLOCK]   = 0b00'1'1,
    [INST_CINVAL_PAGE]    = 0b01'1'1,
    [INST_CINVAL_ALL]     = 0b10'1'1,
    [INST_CINVAL_I_BLOCK] = 0b00'1'0,
    [INST_CINVAL_I_PAGE]  = 0b01'1'0,
    [INST_CINVAL_I_ALL]   = 0b10'1'0,
    [INST_CINVAL_D_BLOCK] = 0b00'0'1,
    [INST_CINVAL_D_PAGE]  = 0b01'0'1,
    [INST_CINVAL_D_ALL]   = 0b10'0'1,
    /* CFETCH             i s l */
    [INST_CFETCH_L]   = 0b0'0'1,
    [INST_CFETCH_S]   = 0b0'1'0,
    [INST_CFETCH_I]   = 0b1'0'0,
    [INST_CFETCH_LS]  = 0b0'1'1,
    [INST_CFETCH_LI]  = 0b1'0'1,
    [INST_CFETCH_SI]  = 0b1'1'0,
    [INST_CFETCH_LSI] = 0b1'1'1,
    /* SSI           C */
    [INST_SSI]   = 0b0,
    [INST_SSI_C] = 0b1,
    /* REV           MASK */
    [INST_REV_Q]   = 0b111000,
    [INST_REV_H]   = 0b110000,
    [INST_REV_B]   = 0b100000,
    [INST_REV_BIT] = 0b111111,
    /* FENCE           S L */
    [INST_FENCE]   = 0b1'1,
    [INST_FENCE_S] = 0b1'0,
    [INST_FENCE_L] = 0b0'1
};

static AphelOpcode inst_name_to_opcode[256] = {
    [INST_LW] = OP_LW,
    [INST_LH] = OP_LH,
    [INST_LQ] = OP_LQ,
    [INST_LB] = OP_LB,
    [INST_LLW] = OP_LLW,
    [INST_LLH] = OP_LLH,
    [INST_LLQ] = OP_LLQ,
    [INST_LLB] = OP_LLB,

    [INST_SW] = OP_SW,
    [INST_SH] = OP_SH,
    [INST_SQ] = OP_SQ,
    [INST_SB] = OP_SB,
    [INST_SCW] = OP_SCW,
    [INST_SCH] = OP_SCH,
    [INST_SCQ] = OP_SCQ,
    [INST_SCB] = OP_SCB,

    [INST_FENCE ... INST_FENCE_L] = OP_FENCE,

    [INST_CINVAL_BLOCK ... INST_CINVAL_D_ALL] = OP_CINVAL,

    [INST_CFETCH_L ... INST_CFETCH_LSI] = OP_CFETCH,

    [INST_SSI ... INST_SSI_C] = OP_SSI,

    [INST_ADD] = OP_ADD,
    [INST_SUB] = OP_SUB,
    [INST_MUL] = OP_MUL,
    [INST_UMULH] = OP_UMULH,
    [INST_IMULH] = OP_IMULH,
    [INST_UDIV] = OP_UDIV,
    [INST_IDIV] = OP_IDIV,
    [INST_UREM] = OP_UREM,
    [INST_IREM] = OP_IREM,

    [INST_ADDI] = OP_ADDI,
    [INST_SUBI] = OP_SUBI,
    [INST_MULI] = OP_MULI,
    [INST_UDIVI] = OP_UDIVI,
    [INST_IDIVI] = OP_IDIVI,
    [INST_UREMI] = OP_UREMI,
    [INST_IREMI] = OP_IREMI,

    [INST_AND] = OP_AND,
    [INST_OR] = OP_OR,
    [INST_NOR] = OP_NOR,
    [INST_XOR] = OP_XOR,

    [INST_ANDI] = OP_ANDI,
    [INST_ORI] = OP_ORI,
    [INST_NORI] = OP_NORI,
    [INST_XORI] = OP_XORI,

    [INST_SL] = OP_SL,
    [INST_USR] = OP_USR,
    [INST_ISR] = OP_ISR,
    [INST_SI_U] = OP_SI,
    [INST_SI_I] = OP_SI,
    [INST_CB] = OP_CB,
    [INST_ROR] = OP_ROL,
    [INST_ROL] = OP_ROR,

    [INST_REV ... INST_REV_BIT] = OP_REV,

    [INST_CSB] = OP_CSB,
    [INST_CLZ] = OP_CLZ,
    [INST_CTZ] = OP_CTZ,
    [INST_EXT] = OP_EXT,
    [INST_DEP] = OP_DEP,

    [INST_SEQ] = OP_SEQ,
    [INST_SULT] = OP_SULT,
    [INST_SILT] = OP_SILT,
    [INST_SULE] = OP_SULE,
    [INST_SILE] = OP_SILE,

    [INST_SEQI] = OP_SEQI,
    [INST_SULTI] = OP_SULTI,
    [INST_SILTI] = OP_SILTI,
    [INST_SULEI] = OP_SULEI,
    [INST_SILEI] = OP_SILEI,

    [INST_BZ] = OP_BZ,
    [INST_BN] = OP_BN,

    [INST_JL] = OP_JL,
    [INST_JLR] = OP_JLR,

    [INST_SYSCALL] = OP_SYSCALL,
    [INST_BREAKPT] = OP_BREAKPT,
    [INST_WAIT] = OP_WAIT,
    [INST_SPIN] = OP_SPIN,
    [INST_IRET] = OP_IRET,
    [INST_LCTRL] = OP_LCTRL,
    [INST_SCTRL] = OP_SCTRL,
};

static inline u32 parse_expr(Parser* p);

/// parse an expression that must be constant on the first pass of the assembler
static u64 parse_const_expr(Parser* p);

static inline void advance(Parser* p) {
    p->cursor += 1;
    p->current = p->tokens[p->cursor];
}

static inline void expect_kind(Parser* p, TokenKind kind) {
    if_unlikely (p->current.kind != kind) {
        // TODO("expected token kind %d", kind);
        parse_error(p, p->cursor, "expected '%s', got %s",
            token_kind_name[kind],
            token_kind_name[p->current.kind]
        );
    }
}

static inline void expect_advance(Parser* p, TokenKind kind) {
    expect_kind(p, kind);
    advance(p);
}

static inline void match(Parser* p, TokenKind kind) {
    if (p->current.kind == kind) {
        advance(p);
    }
}

// returns -1 if cannot be found.
static isize symbol_find(Parser* p, string ident) {
    void* index = strmap_get(&p->symbol_indexes, ident);

    if (index != STRMAP_NOT_FOUND) {
        return (isize) index;
    }

    return -1;
}

// returns index into symbol table
static usize symbol_find_or_create_undef(Parser* p, string ident) {
    void* index = strmap_get(&p->symbol_indexes, ident);

    if (index != STRMAP_NOT_FOUND) {
        return (isize) index;
    }

    assert(ident.len <= UINT16_MAX);

    // CREATEAEEEEEEEEEEE

    Symbol symbol = {
        .bind = SYM_UNDEFINED,
        .name = ident.raw,
        .name_len = ident.len,
        .section_def = p->current_section_index,
    };

    usize new_index = vec_len(p->symbols);
    vec_append(&p->symbols, symbol);
    strmap_put(&p->symbol_indexes, ident, (void*)new_index);

    return new_index;
}

static void element_add(Parser* p, SectionElement e, u32 token_index) {
    vec_append(&p->current_section->elements, e);
    vec_append(&p->current_section->elem_tokens, token_index);
}

#define WRAP_SPAN(span, stmt) TokenSpan span = {p->cursor, 0}; stmt; span.end = p->cursor

static string parse_strlit_contents(Parser* p) {
    expect_kind(p, TOK_STR_LIT);

    Arena* arena = &p->luna->permanent;

    Token t = p->current;

    if (t.subkind == TOK_STR_HAS_ESCAPE) {
        TODO("handle escape sequences in string");
    } else {
        const char* content_start = &token_start(t)[1];
        const char* content_end = strchr(content_start, '\"');

        advance(p);
        return (string){
            .raw = (char*) content_start,
            .len = content_end - content_start,
        };
    }
}

/// Check if the character `c` can be in the middle of an identifier.
static bool is_ident_middle(char c) {
    switch (c) {
    case '.':
    case '_':
    case 'a' ... 'z':
    case 'A' ... 'Z':
    case '0' ... '9':
        return true;
    default:
        return false;
    }
}

static usize compute_ident_len(const char* start) {
    usize len = 1;
    while (is_ident_middle(start[len])) {
        len++;
    }
    return len;
}

static ComplexExpr* cexpr(Parser* p, u32 index) {
    return &p->exprs[index];
}

i64 int_in_bits(i64 n, usize bits) {
    return ((n << (64 - bits)) >> (64 - bits));
}

u64 uint_in_bits(u64 n, usize bits) {
    return ((n << (64 - bits)) >> (64 - bits));
}

static u32 cexpr_create(Parser* p, ComplexExprKind kind) {
    u32 index = vec_len(p->exprs);
    _vec_reserve1((void**)&p->exprs, vec_stride(p->exprs));
    cexpr(p, index)->kind = kind;
    cexpr(p, index)->token_index = p->cursor;
    vec_len(p->exprs) += 1;
    return index;
}

static void parse_label_or_symbol(Parser* p) {
    // label       = {symbol_modifier} [symbol_binding] symbol ":" nl
    // symbol_decl = {symbol_modifier} [symbol_binding] "symbol" nl;

    bool is_entry = false;
    SymbolBind bind = SYM_LOCAL;

    if (p->current.kind == TOK_KW_ENTRY) {
        is_entry = true;
        advance(p);
    }

    switch (p->current.kind) {
    case TOK_KW_GLOBAL:
        bind = SYM_GLOBAL;
        advance(p);
        break;
    case TOK_KW_LOCAL:
        bind = SYM_LOCAL;
        advance(p);
        break;
    case TOK_KW_WEAK:
        bind = SYM_WEAK;
        advance(p);
        break;
    default:
        break;
    }

    usize ident_index = p->cursor;
    string ident;
    switch (p->current.kind) {
    case TOK_KW_SYMBOL:
        TODO("handle 'symbol' declaration");
    case TOK_STR_LIT:
        ident = parse_strlit_contents(p);
        break;
    case TOK_IDENT:
        ident.raw = (char*)token_start(p->current);
        ident.len = compute_ident_len(ident.raw);
        advance(p);
        break;
    }
    expect_advance(p, TOK_COLON);

    if (ident.len > UINT16_MAX) {
        parse_error(p, ident_index, "symbol too long (max %d)", UINT16_MAX);
    }

    usize sym_index = symbol_find_or_create_undef(p, ident);
    Symbol* sym = &p->symbols[sym_index];
    if (sym->bind != SYM_UNDEFINED) {
        parse_error(p, ident_index, "symbol is already defined", UINT16_MAX);
    }
    sym->bind = bind;
    sym->section_def = p->current_section_index;

    SectionElement label = { .label = {
        ._kind = ELEM_LABEL,
        .symbol_index = sym_index,
    }};

    element_add(p, label, ident_index);
}

static void parse_directive_align(Parser* p) {
    expect_advance(p, TOK_KW_ALIGN);
    WRAP_SPAN(span, u64 value = parse_const_expr(p));
    expect_advance(p, TOK_NEWLINE);

    if (!is_pow_2(value)) {
        parse_error(p, span.start, "%llu is not a power of two", value);
    }

    if (value > (1ull << p->current_section->alignment_p2)) {
        parse_warn(p, span.start, "%llu is greater than section alignment", value);
    }

    SectionElement align = { .align = {
        ._kind = ELEM_ALIGN,
        .alignment_p2 = __builtin_ctzll(value),
    }};

    element_add(p, align, span.start - 1);
}

static void parse_directive_string(Parser* p) {
    expect_advance(p, TOK_KW_STRING);
    WRAP_SPAN(span, string contents = parse_strlit_contents(p));
    expect_advance(p, TOK_NEWLINE);

    SectionElement string_1 = { .string = {
        ._kind = ELEM_STRING,
        .length = contents.len,
    }};
    SectionElement string_2 = {
        .supp_string = contents.raw,
    };

    element_add(p, string_1, span.start);
    element_add(p, string_2, span.start);
}

// static void parse_directive_data(Parser* p) {
//     TokenKind kind = p->current.kind;

//     SectionElement value;
//     value.kind = ELEM_WORD;
//     usize value_bits = 64;

//     switch (kind) {
//     case TOK_KW_BYTE:
//         value_bits = 8;
//         value.kind = ELEM_BYTE;
//         break;
//     case TOK_KW_QWORD:
//         value_bits = 16;
//         value.kind = ELEM_QWORD;
//         break;
//     case TOK_KW_HWORD:
//         value_bits = 32;
//         value.kind = ELEM_HWORD;
//         break;
//     default:
//         break;
//     }

//     advance(p);
//     u32 expr_index = parse_expr(p);
//     u64 expr_val = 0;

//     if (cexpr(p, expr_index)->kind == CEXPR_VALUE) {
//         // extract the constant value and encode it directly
//         expr_val = cexpr(p, expr_index)->value;
//         if (expr_val != int_in_bits(expr_val, value_bits)) {
//             parse_warn(p, cexpr(p, expr_index)->token_index,
//                 "constant expression is truncated to %d",
//                 int_in_bits(value_bits, 9)
//             );
//         }
//     } else {
//         // imm_expr_elem.kind = ELEM_EXPR;
//         // imm_expr_elem.expr.index = imm_expr;
//     }
// }

static bool parse_sec_element(Parser* p) {

    while_unlikely (p->current.kind == TOK_NEWLINE) {
        advance(p);
    }

    switch (p->current.kind) {
    case TOK_INST:
        parse_instruction(p);
        return true;

    case TOK_KW_ENTRY:
    case TOK_KW_GLOBAL:
    case TOK_KW_LOCAL:
    case TOK_KW_WEAK:
    case TOK_STR_LIT:
    case TOK_IDENT:
        parse_label_or_symbol(p);
        return true;
    case TOK_KW_ALIGN:
        parse_directive_align(p);
        return true;
    case TOK_KW_STRING:
        parse_directive_string(p);
        return true;
    default:
        return false;
    }
}

/// Check if the character `c` can be in the middle of a numeric constant.
static bool is_numeric_middle(char c) {
    switch (c) {
    case '_':
    case 'a' ... 'z':
    case 'A' ... 'Z':
    case '0' ... '9':
        return true;
    default:
        return false;
    }
}

static u64 parse_numeric_literal(Parser* p) {
    const char* begin = token_start(p->current);
    advance(p);

    if (!is_numeric_middle(begin[1])) {
        return begin[0] - '0';
    }

    u64 base = 10;
    if (begin[0] == '0') {
        switch (begin[1]) {
        case 'x':
        case 'X':
            base = 16;
            begin = &begin[2];
            break;
        case 'o':
        case 'O':
            base = 8;
            begin = &begin[2];
            break;
        case 'b':
        case 'B':
            base = 2;
            begin = &begin[2];
            break;
        case '0' ... '9':
            break;
        default:
            parse_error_at_ptr(p, &begin[1], "invalid base specifier");
        }
    }

    u64 value = 0;
    for (usize i = 0;; i++) {
        char c = begin[i];
        if (!is_numeric_middle(c)) {
            break;
        }

        u64 c_value;
        switch (c) {
        case '_':
            continue;
        case 'a' ... 'f':
            c_value = c - 'a' + 10;
            break;
        case 'A' ... 'F':
            c_value = c - 'A' + 10;
            break;
        case '0' ... '9':
            c_value = c - '0';
            break;
        default:
            c_value = 100000;
        }

        if_unlikely (c_value > base) {
            parse_error_at_ptr(p, &begin[i], "invalid base %d digit", base);
        }

        if_unlikely (ckd_mul(&value, value, base)) {
            parse_error_at_ptr(p, &begin[i], "numeric literal is too large", begin[1]);
        }

        if_unlikely (ckd_add(&value, value, c_value)) {
            parse_error_at_ptr(p, &begin[i], "numeric literal is too large", begin[1]);
        }
    }
    return value;
}

/// Parses an expression and returns an index into `p->exprs`
static u32 parse_expr(Parser* p);

static u32 parse_atom(Parser* p) {
    switch (p->current.kind) {
    case TOK_IDENT: {
        string ident;
        ident.raw = (char*)token_start(p->current);
        ident.len = compute_ident_len(ident.raw);

        if (ident.len > UINT16_MAX) {
            parse_error(p, p->cursor, "symbol too long (max %d)", UINT16_MAX);
        }

        u32 symbol_index = symbol_find_or_create_undef(p, ident);

        u32 expr_index = cexpr_create(p, CEXPR_SYMBOL_REF);
        cexpr(p, expr_index)->symbol_ref = symbol_index;

        advance(p);
        return expr_index;
    }
    case TOK_OPEN_PAREN: {
        advance(p);
        u32 inner = parse_expr(p);
        expect_advance(p, TOK_CLOSE_PAREN);
        return inner;
    }
    case TOK_NUM_LIT: {
        u64 value = parse_numeric_literal(p);
        u32 index = cexpr_create(p, CEXPR_VALUE);
        cexpr(p, index)->value = value;
        return index;
    }
    default:
        parse_error(p, p->cursor, "expected expression");
        UNREACHABLE;
    }
}

static u32 parse_unary(Parser* p) {
    switch (p->current.kind) {
    case TOK_PLUS:
        advance(p);
        return parse_unary(p);
    case TOK_MINUS: {
        u32 token_index = p->cursor;

        advance(p);
        u32 index = parse_unary(p);
        ComplexExpr* inner = cexpr(p, index);

        // eagerly evaluate, don't create a new node if we don't have to
        if (inner->kind == CEXPR_VALUE) {
            inner->value = -inner->value;
        } else {
            u32 outer = cexpr_create(p, CEXPR_NEG);
            cexpr(p, outer)->un = index;
            index = outer;
        }
        cexpr(p, index)->token_index = token_index;
        return index;
    }
    case TOK_TILDE: {
        u32 token_index = p->cursor;

        advance(p);
        u32 index = parse_unary(p);
        ComplexExpr* inner = cexpr(p, index);

        // eagerly evaluate, don't create a new node if we don't have to
        if (inner->kind == CEXPR_VALUE) {
            inner->value = ~inner->value;
        } else {
            u32 outer = cexpr_create(p, CEXPR_NOT);
            cexpr(p, outer)->un = index;
            index = outer;
        }
        cexpr(p, index)->token_index = token_index;
        return index;
    }
    default:
        return parse_atom(p);
    }
}

static isize bin_precedence(TokenKind kind) {
    switch (kind) {
    case TOK_MUL:
    case TOK_DIV:
    case TOK_REM:
        return 4;
    case TOK_PLUS:
    case TOK_MINUS:
        return 3;
    case TOK_AND:
        return 2;
    case TOK_OR:
        return 1;
    default:
        return -1;
    }
}

static u32 parse_binary(Parser* p, isize precedence) {
    u32 lhs = parse_unary(p);

    usize op_index = p->cursor;
    Token operator = p->current;
    isize op_prec = bin_precedence(operator.kind);
    while (precedence < op_prec) {
        advance(p);
        usize cexpr_array_len = vec_len(p->exprs);
        u32 rhs = parse_binary(p, op_prec);

        ComplexExpr* lhs_expr = cexpr(p, lhs);
        ComplexExpr* rhs_expr = cexpr(p, rhs);

        bool can_const_eval =
            cexpr(p, lhs)->kind == CEXPR_VALUE &&
            cexpr(p, rhs)->kind == CEXPR_VALUE
        ;

        switch (operator.kind) {
        case TOK_PLUS:
            if (can_const_eval) {
                if (ckd_add(&lhs_expr->value, lhs_expr->value, rhs_expr->value)) {
                    parse_warn(p, op_index, "constant overflows to %llu (%lli)",
                        lhs_expr->value,
                        lhs_expr->value
                    );
                }
            } else {
                u32 op = cexpr_create(p, CEXPR_ADD);
                cexpr(p, op)->bin.lhs = lhs;
                cexpr(p, op)->bin.rhs = rhs;
                lhs = op;
            }
            break;
        case TOK_MINUS:
            if (can_const_eval) {
                if (ckd_sub(&lhs_expr->value, lhs_expr->value, rhs_expr->value)) {
                    parse_warn(p, op_index, "constant underflows to %llu (%lli)",
                        lhs_expr->value,
                        lhs_expr->value
                    );
                }
            } else {
                u32 op = cexpr_create(p, CEXPR_SUB);
                cexpr(p, op)->bin.lhs = lhs;
                cexpr(p, op)->bin.rhs = rhs;
                lhs = op;
            }
            break;
        case TOK_MUL:
            if (can_const_eval) {
                if (ckd_mul(&lhs_expr->value, lhs_expr->value, rhs_expr->value)) {
                    parse_warn(p, op_index, "constant overflows to %llu (%lli)",
                        lhs_expr->value,
                        lhs_expr->value
                    );
                }
            } else {
                u32 op = cexpr_create(p, CEXPR_MUL);
                cexpr(p, op)->bin.lhs = lhs;
                cexpr(p, op)->bin.rhs = rhs;
                lhs = op;
            }
            break;
        case TOK_DIV:
            if (can_const_eval) {
                if (rhs_expr->value == 0) {
                    parse_error(p, op_index, "constant divisor is zero",
                        lhs_expr->value
                    );
                }
                lhs_expr->value = lhs_expr->value / rhs_expr->value;
            } else {
                u32 op = cexpr_create(p, CEXPR_DIV);
                cexpr(p, op)->bin.lhs = lhs;
                cexpr(p, op)->bin.rhs = rhs;
                lhs = op;
            }
            break;
        case TOK_REM:
            if (can_const_eval) {
                if (rhs_expr->value == 0) {
                    parse_error(p, op_index, "constant divisor is zero",
                        lhs_expr->value
                    );
                }
                lhs_expr->value = lhs_expr->value % rhs_expr->value;
            } else {
                u32 op = cexpr_create(p, CEXPR_REM);
                cexpr(p, op)->bin.lhs = lhs;
                cexpr(p, op)->bin.rhs = rhs;
                lhs = op;
            }
            break;
        case TOK_AND:
            if (can_const_eval) {
                lhs_expr->value = lhs_expr->value & rhs_expr->value;
            } else {
                u32 op = cexpr_create(p, CEXPR_AND);
                cexpr(p, op)->bin.lhs = lhs;
                cexpr(p, op)->bin.rhs = rhs;
                lhs = op;
            }
            break;
        case TOK_OR:
            if (can_const_eval) {
                lhs_expr->value = lhs_expr->value | rhs_expr->value;
            } else {
                u32 op = cexpr_create(p, CEXPR_OR);
                cexpr(p, op)->bin.lhs = lhs;
                cexpr(p, op)->bin.rhs = rhs;
                lhs = op;
            }
            break;
        default:
            TODO("invalid operator");
        }

        cexpr(p, lhs)->token_index = op_index;

        // recycle expression space
        if (can_const_eval) {
            vec_len(p->exprs) = cexpr_array_len;
        }

        op_index = p->cursor;
        operator = p->current;
        op_prec = bin_precedence(operator.kind);
    }

    return lhs;
}

static inline u32 parse_expr(Parser* p) {
    return parse_binary(p, 0);
}

/// parse an expression that must be constant on the first pass of the assembler
static u64 parse_const_expr(Parser* p) {

    usize cexpr_array_len = vec_len(p->exprs);

    usize start_index = p->cursor;
    u32 e = parse_expr(p);
    ComplexExpr* expr = cexpr(p, e);

    if (expr->kind != CEXPR_VALUE) {
        parse_error(p, start_index, "expression must be constant on first pass");
    }

    vec_len(p->exprs) = cexpr_array_len;

    return expr->value;
}

static inline AphelGpr parse_operand_gpr(Parser* p) {
    AphelGpr reg = p->current.subkind;
    expect_advance(p, TOK_GPR);
    return reg;
}

static inline u32 parse_operand_ctrl(Parser* p) {
    u32 reg = p->current.subkind;
    expect_advance(p, TOK_CTRL);
    return reg;
}

/// An operand representing a memory location, like `[r1 + r2 + imm]`
typedef struct MemOperand {
    AphelGpr r1;
    AphelGpr r2;
    u64 addend;
} MemOperand;

/// Parse a memory operand:
/// - `[r1]`
/// - `[r1 + imm]`
/// - `[r1 + r2]`
/// - `[r1 + r2 + imm]`
/// \param no_r2 if true, forms with `r2` provided are invalid.
static MemOperand parse_mem_operand(Parser* p, bool no_r2) {
    expect_advance(p, TOK_OPEN_BRACKET);

    MemOperand mem = {
        .r1 = GPR_ZR,
        .r2 = GPR_ZR,
        .addend = 0,
    };

    mem.r1 = parse_operand_gpr(p);

    if (p->current.kind == TOK_PLUS) {
        advance(p);
        if (p->current.kind == TOK_GPR) {
            if (no_r2) {
                parse_error(p, p->cursor, "this instruction only accepts one register for addressing");
            }
            mem.r2 = p->current.subkind;
            advance(p);
            if (p->current.kind == TOK_PLUS) {
                advance(p);
                mem.addend = parse_const_expr(p);
            }
        } else {
            mem.addend = parse_const_expr(p);
        }
    }

    expect_advance(p, TOK_CLOSE_BRACKET);

    return mem;
}

typedef enum : u8 {
    OPERAND_CONST_EXPR,
    OPERAND_MEM_NO_R2,
    OPERAND_MEM,
    OPERAND_EXPR,
    OPERAND_GPR,
    OPERAND_CTRL,
    OPERAND_QUARTER,
} OperandKind;

static u8 parse_n_ops(Parser* p, u8 n, u32 op_kind, ...) {
    if (p->current.kind == TOK_NEWLINE)
        return 0;

    OperandKind kind = (OperandKind) op_kind; // last must be non-promotable
    va_list outs;
    va_start(outs, op_kind);

    u8 i;

    for (i = 0; i < n; i++) {
        switch (kind) {
        case OPERAND_MEM: {
            AphelGpr* r1 = va_arg(outs, AphelGpr*);
            AphelGpr* r2 = va_arg(outs, AphelGpr*);
            u64* addend  = va_arg(outs, u64*);
            
            MemOperand m = parse_mem_operand(p, false);
            *r1 = m.r1;
            *r2 = m.r2;
            *addend = m.addend;
        } break;
        case OPERAND_MEM_NO_R2: {
            AphelGpr* r1 = va_arg(outs, AphelGpr*);
            u64* addend  = va_arg(outs, u64*);
            
            MemOperand m = parse_mem_operand(p, true);
            *r1 = m.r1;
            *addend = m.addend;
        } break;
        case OPERAND_CONST_EXPR: {
            usize* tok = va_arg(outs, usize*);
            u64* value = va_arg(outs, u64*);

            *tok = p->cursor;
            *value = parse_const_expr(p);
        } break;
        case OPERAND_EXPR: {
            usize* tok = va_arg(outs, usize*);
            u32* expr_index = va_arg(outs, u32*);

            *tok = p->cursor;
            *expr_index = parse_expr(p);
        } break;
        case OPERAND_CTRL: {
            AphelCtrl* ctrl = va_arg(outs, AphelCtrl*);
            *ctrl = parse_operand_ctrl(p);
        } break;
        case OPERAND_GPR: {
            AphelGpr* r = va_arg(outs, AphelGpr*);

            if (p->current.kind != TOK_GPR)
                break;
            *r = parse_operand_gpr(p);
        } break;
        case OPERAND_QUARTER: {
            u8* quarter = va_arg(outs, u8*);
            u64 v = parse_const_expr(p);
            
            if (v % 16 != 0 || v > 48) {
                parse_error(p, p->cursor, "SSI quarter must be 0, 16, 32, or 48. Got: %d", v);
            }

            *quarter = (u8)(v / 16);
        } break;
        default:
            UNREACHABLE;
        }
        match(p, TOK_COMMA);
    }

    va_end(outs);
    return i;
}

static void check_imm(Parser* p, u8 subkind, u32 imm_expr, u8 imm_size, usize expr_start_token) {
    if (cexpr(p, imm_expr)->kind != CEXPR_VALUE)
        return;

    // extract the constant value and encode it directly
    u64 imm = cexpr(p, imm_expr)->value;
    if (inst_name_imm_signed[subkind]) {
        if (imm != int_in_bits(imm, imm_size)) {
            parse_warn(p, expr_start_token,
                "constant expression is truncated to %d (%x)",
                int_in_bits(imm, imm_size),
                int_in_bits(imm, imm_size)
            );
        }
    } else {
        if (imm != uint_in_bits(imm, imm_size)) {
            parse_warn(p, expr_start_token,
                "constant expression is truncated to %u (%x)",
                uint_in_bits(imm, imm_size),
                uint_in_bits(imm, imm_size)
            );
        }
    }
}

static void parse_instruction(Parser* p) {
    Token inst = p->current;
    usize inst_start_token = p->cursor;

    expect_advance(p, TOK_INST);
    AphelGpr r1 = GPR_ZR;
    AphelGpr r2 = GPR_ZR;
    AphelGpr r3 = GPR_ZR;

    usize imm_start_token;
    SectionElement imm_expr_elem = {0};
    imm_expr_elem.kind = ELEM_EXPR;

    u32 imm = 0;

    // in the case of inlining, how much to shift the immediate by
    u8 imm_shift = 0; 
    // acts as a pseudo-value for the immediate
    u64 val = 0;

    u8 pseudo_inst_size = 0;

    switch (inst.subkind) {
    case INST_LW ... INST_LLB:
        char* addr_type = "source";
        parse_n_ops(p, 1, OPERAND_GPR, &r1); // src
        parse_n_ops(p, 1, OPERAND_MEM, &r2, &r3, &val); // dest
        goto alignment_check;
    case INST_SCW ... INST_SCB: 
        addr_type = "destination";
        parse_n_ops(p, 1, OPERAND_GPR, &r2); // cond
        parse_n_ops(p, 1, OPERAND_MEM_NO_R2, &r3, &val); // dest
        parse_n_ops(p, 1, OPERAND_GPR, &r1); // src
        goto alignment_check;
    case INST_SW ... INST_SB:
        addr_type = "destination";
        parse_n_ops(p, 1, OPERAND_MEM, &r2, &r3, &val); // dest
        parse_n_ops(p, 1, OPERAND_GPR, &r1); // src
    alignment_check: {
        u8 alignment;
        switch (inst.subkind) {
        case INST_LW:
        case INST_LLW:
        case INST_SW:
        case INST_SCW:
            alignment = 8;
            break;
        case INST_LH:
        case INST_LLH:
        case INST_SH:
        case INST_SCH:
            alignment = 4;
            break;
        case INST_LQ:
        case INST_LLQ:
        case INST_SQ:
        case INST_SCQ:
            alignment = 2;
            break;
        case INST_LB:
        case INST_LLB:
        case INST_SB:
        case INST_SCB:
            alignment = 1;
            break;
        default:
            UNREACHABLE;
        }

        if (val % alignment != 0) {
            val /= alignment;
            parse_warn(p, p->cursor, "%s address must be %d-byte aligned, rounded to %d (%x).", addr_type, alignment, val * alignment, val * alignment);
        } else {
            val /= alignment;
        }
    } break;
    case INST_FENCE ... INST_FENCE_L: {
        val = inst_to_imm[inst.subkind];
    } break;
    case INST_CINVAL_BLOCK ... INST_CINVAL_D_ALL: {
        val = inst_to_imm[inst.subkind];
        // if all is not set, parse register
        if ((val & 0b11'0'0) != 0b10'0'0 && parse_n_ops(p, 1, OPERAND_GPR, &r1) != 1) {
            parse_error(p, p->cursor, "expected register operand to determine the associated %s to invalidate.",
                   ((val & 0b11'0'0) >> 2) == 0b00 ? "cache block" : "pages" );
        }
    } break;
    case INST_CFETCH_L ... INST_CFETCH_LSI: {
        val = inst_to_imm[inst.subkind];
        if (parse_n_ops(p, 1, OPERAND_GPR, &r1) != 1)
            parse_error(p, p->cursor, "expected register operand to determine the associated cache block to fetch.");
    } break;
    case INST_SSI ... INST_SSI_C: {
        imm_shift = 3;
        u8 sh;

        val = inst_to_imm[inst.subkind]; // set clear

        if (parse_n_ops(p, 1, OPERAND_GPR, &r1) != 1) // dest
            parse_error(p, p->cursor, "expected destination register.");

        if (parse_n_ops(p, 1, OPERAND_EXPR, &imm_start_token, &imm_expr_elem.expr.index) != 1) // expr
            parse_error(p, p->cursor, "expected expression (source).");
        check_imm(p, inst.subkind, imm, 16, imm_start_token);

        if (parse_n_ops(p, 1, OPERAND_QUARTER, &sh) != 1) // dest
            parse_error(p, p->cursor, "expected quarter (0, 16, 32, 48) to load at.");
        val |= sh << 1;
    } break;
    case INST_ADD ... INST_XOR: {
        switch (parse_n_ops(p, 3, OPERAND_GPR, &r1, &r2, &r3)) {
        case 0:
            parse_error(p, p->cursor, "expected destination register.");
            break;
        case 1:
            parse_error(p, p->cursor, "expected register operand (lhs).");
            break;
        default:
            break;
        }

        parse_n_ops(p, 1, OPERAND_EXPR, &imm_start_token, &imm_expr_elem.expr.index);
        check_imm(p, inst.subkind, imm, 9, imm_start_token);
    } break;
    case INST_ADDI ... INST_XORI: {
        switch (parse_n_ops(p, 2, OPERAND_GPR, &r1, &r2)) {
        case 0:
            parse_error(p, p->cursor, "expected destination register.");
            break;
        case 1:
            parse_error(p, p->cursor, "expected register operand (lhs).");
            break;
        default:
            break;
        }

        parse_n_ops(p, 1, OPERAND_EXPR, &imm_start_token, &imm_expr_elem.expr.index);
        check_imm(p, inst.subkind, imm, 9, imm_start_token);
    } break;
    case INST_SL ... INST_ISR:
    case INST_ROR ... INST_ROL: {
        switch (parse_n_ops(p, 3, OPERAND_GPR, &r1, &r2, &r3)) {
        case 0:
            parse_error(p, p->cursor, "expected destination register.");
            break;
        case 1:
            parse_error(p, p->cursor, "expected operand register(s).");
            break;
        default:
            break;
        }

        parse_n_ops(p, 1, OPERAND_EXPR, &imm_start_token, &imm_expr_elem.expr.index);
        check_imm(p, inst.subkind, imm, 9, imm_start_token);
        if (imm > 63)
            parse_warn(p, imm_start_token, "expression wrapped to %d (%x) as shifts are mod 64", imm, imm);
    } break;
    case INST_SI_U ... INST_CB: {
        val |= (inst.subkind == INST_SI_I) << 12;

        switch (parse_n_ops(p, 1, OPERAND_GPR, &r1, &r2, &r3)) {
        case 0:
            parse_error(p, p->cursor, "expected destination register.");
            break;
        case 1:
            parse_error(p, p->cursor, "expected operand register(s).");
            break;
        default:
            break;
        }

        TODO("Implement rsh and lsh"); // The imm_shift would be interesting here
        parse_n_ops(p, 1, OPERAND_EXPR, &imm_start_token, &imm_expr_elem.expr.index);
        check_imm(p, inst.subkind, imm, 6, imm_start_token);
        break;
    }
    case INST_REV ... INST_REV_BIT: {
        switch (parse_n_ops(p, 2, OPERAND_GPR, &r1, &r2)) {
            case 0:
                parse_error(p, p->cursor, "expected destination register.");
                break;
            case 1:
                parse_error(p, p->cursor, "expected register operand.");
                break;
            default:
                break;
        }

        if (inst.subkind == INST_REV) {
            if (parse_n_ops(p, 1, OPERAND_EXPR, &imm_start_token, &imm_expr_elem.expr.index) != 1)
                parse_error(p, imm_start_token, "expected an 6-bit bitflag expression of chunks to reverse.");

            check_imm(p, inst.subkind, imm, 9, imm_start_token);
            if (imm > 63) {
                // TODO: word this error better
                parse_warn(p, imm_start_token, "cannot reverse chunks larger than 32 bits, yet higher flags were set: %d (%x)", imm, imm);
            }
            break;
        } else {
            val = inst_to_imm[inst.subkind];
        }
    } break;
    case INST_CSB ... INST_CTZ: {
        switch (parse_n_ops(p, 2, OPERAND_GPR, &r1, &r2)) {
            case 0:
                parse_error(p, p->cursor, "expected destination register.");
                break;
            case 1:
                parse_error(p, p->cursor, "expected register operand.");
                break;
            default:
                break;
        }
    } break;
    case INST_EXT ... INST_DEP: {
        switch (parse_n_ops(p, 3, OPERAND_GPR, &r1, &r2, &r3)) {
            case 0:
                parse_error(p, p->cursor, "expected destination register.");
                break;
            case 1:
                parse_error(p, p->cursor, "expected register operand (source).");
                break;
            case 2:
                parse_error(p, p->cursor, "expected register operand (mask).");
                break;
            default:
                break;
        }
    } break;
    case INST_SEQ ... INST_SILE: {
        switch (parse_n_ops(p, 3, OPERAND_GPR, &r1, &r2, &r3)) {
            case 0:
                parse_error(p, p->cursor, "expected destination register.");
                break;
            case 1:
                parse_error(p, p->cursor, "expected register operand (lhs).");
                break;
            case 2:
                parse_error(p, p->cursor, "expected register operand (rhs).");
                break;
            default:
                break;
        }

        parse_n_ops(p, 1, OPERAND_EXPR, &imm_start_token, &imm_expr_elem.expr.index);
        check_imm(p, inst.subkind, imm, 9, imm_start_token);
    } break;
    case INST_SEQI ... INST_SILEI: {
        switch (parse_n_ops(p, 3, OPERAND_GPR, &r1, &r2, &r3)) {
            case 0:
                parse_error(p, p->cursor, "expected destination register.");
                break;
            case 1:
                parse_error(p, p->cursor, "expected register operand (lhs).");
                break;
        }

        if (parse_n_ops(p, 1, OPERAND_EXPR, &imm_start_token, &imm_expr_elem.expr.index) != 1)
            parse_error(p, imm_start_token, "expected expression operand (rhs).");
        check_imm(p, inst.subkind, imm, 9, imm_start_token);
    } break;
    case INST_BZ ... INST_BN: {
        if (parse_n_ops(p, 1, OPERAND_GPR, &r1) != 1)
                parse_error(p, p->cursor, "expected register operand for comparison.");

        if (parse_n_ops(p, 1, OPERAND_EXPR, &imm_start_token, &imm_expr_elem.expr.index) != 1)
            parse_error(p, p->cursor, "expected expression operand (address).");
        check_imm(p, inst.subkind, imm, 19, imm_start_token);

        if (imm % 4 != 0)
            parse_warn(p, imm_start_token, "relative address must be 4-byte aligned, trunctated to %d (%x).", imm, imm);
    } break;
    case INST_JL ... INST_JLR: {
        switch (parse_n_ops(p, 2, OPERAND_GPR, &r1, &r2)) {
            case 0:
                parse_error(p, p->cursor, "expected destination (link) register.");
                break;
            case 1:
                parse_error(p, p->cursor, "expected register operand (address base).");
                break;
            default:
                break;
        }
        parse_n_ops(p, 1, OPERAND_EXPR, &imm_start_token, &imm_expr_elem.expr.index);
        check_imm(p, inst.subkind, imm, 14, imm_start_token); 
        if (imm % 4 != 0)
            parse_warn(p, imm_start_token, "relative address must be 4-byte aligned, trunctated to %d (%x).", imm, imm);
    } break;
    case INST_BREAKPT ... INST_IRET:
        break;
    case INST_LCTRL: {
        if (parse_n_ops(p, 1, OPERAND_GPR, &r1) != 1)
            parse_error(p, p->cursor, "expected destination register (GPR).");
        if (parse_n_ops(p, 1, OPERAND_CTRL, &imm) != 1)
            parse_error(p, p->cursor, "expected source register (Control).");
    } break;
    case INST_SCTRL:
        if (parse_n_ops(p, 1, OPERAND_CTRL, &imm) != 1)
            parse_error(p, p->cursor, "expected destination register (Control).");
        if (parse_n_ops(p, 1, OPERAND_GPR, &r1) != 1)
            parse_error(p, p->cursor, "expected source register (GPR).");
        break;
    case INST_P_NOP:
        break;
    case INST_P_RET:
        if (parse_n_ops(p, 1, OPERAND_GPR, &r1) != 1) {
            r1 = GPR_LP;
        }
        break;
    case INST_P_CALL:
    case INST_P_FCALL: {
        pseudo_inst_size = max_inst_size(inst.subkind);
        r1 = GPR_LP; // default semantics differ
        r2 = GPR_LP;

        if (parse_n_ops(p, 2, OPERAND_GPR, &r1, &r2) == 1)
            r2 = r1;

        if (parse_n_ops(p, 1, OPERAND_EXPR, &imm_start_token, &imm_expr_elem.expr.index) != 1)
            parse_error(p, imm_start_token, "expected expression operand (address).");
    } break;
    case INST_P_LI: {
        pseudo_inst_size = max_inst_size(inst.subkind);
        if (parse_n_ops(p, 1, OPERAND_GPR, &r1) != 1)
                parse_error(p, p->cursor, "expected destination register.");
        if (parse_n_ops(p, 1, OPERAND_EXPR, &imm_start_token, &imm_expr_elem.expr.index) != 1)
            parse_error(p, imm_start_token, "expected expression operand (source).");
    } break;
    case INST_P_MOV: {
        pseudo_inst_size = max_inst_size(inst.subkind);
        switch (parse_n_ops(p, 2, OPERAND_GPR, &r1, &r2)) {
            case 0:
                parse_error(p, p->cursor, "expected destination register.");
                break;
            case 1:
                parse_error(p, p->cursor, "expected source register.");
                break;
            default:
                break;
        }
    } break;
    default:
        break;
    }
    expect_advance(p, TOK_NEWLINE);
    
    // eagerly evaluate if possible
    if (cexpr(p, imm_expr_elem.expr.index)->kind == CEXPR_VALUE) {
        val |= cexpr(p, imm_expr_elem.expr.index)->value << imm_shift;
    }

    SectionElement elem = { .inst = {
        .name = inst.subkind,
        .r1 = r1,
        .r2 = r2,
        .r3 = r3,
        .imm = val
    }};

    if (pseudo_inst_size != 0) {
        elem.pseudo_inst.size = pseudo_inst_size;
    }

    element_add(p, elem, inst_start_token);

    if (cexpr(p, imm_expr_elem.expr.index)->kind != CEXPR_VALUE) {
        printf("expr made \n");
        element_add(p, imm_expr_elem, imm_start_token);
    }
}

u64 max_inst_size(InstName instname) {
    switch (instname) {
    case INST_P_CALL:
        return 8;
    case INST_P_FCALL:
        return 16;
    case INST_P_LI:
        return 16;
    default:
        return 4;
    }
}

static void parse_section(Parser* p, ApoSectionFlags flags) {
    Arena* arena = &p->luna->permanent;

    expect_advance(p, TOK_KW_SECTION);
    string section_name = parse_strlit_contents(p);

    // default alignment
    u64 alignment = 8;

    while (true) {
        if (p->current.kind == TOK_KW_ALIGN) {
            // section-level align
            advance(p);
            usize index = p->cursor;
            alignment = parse_const_expr(p);
            if (!is_pow_2(alignment)) {
                parse_error(p, index, "section alignment is not a power of two");
            }
            if (alignment < 8) {
                parse_warn(p, index, "section alignment is less than the default (8)");
            }
            continue;
        } else {
            break;
        }
    }

    expect_advance(p, TOK_NEWLINE);

    // create a new section.
    Section* section = arena_alloc(arena, sizeof(Section), alignof(Section));
    *section = (Section){
        .name = section_name.raw,
        .name_len = section_name.len,
        .alignment_p2 = __builtin_ctzll(alignment),
        .flags = flags,
        .relocs = vec_new(Relocation, 64),
        .elements = vec_new(SectionElement, 64),
        .elem_tokens = vec_new(u32, 64),
    };

    if (p->current_section != nullptr) {
        p->current_section_index += 1;
    }

    vec_append(&p->sections, section);
    p->current_section = section;

    while (parse_sec_element(p)) {}

    // add skip element at the end so that significant elements can safely
    // query for an expression placed after it
    element_add(p, (SectionElement){
        .kind = ELEM_SKIP,
    }, p->cursor);

}

void parse_section_flags(Parser* p) {
    // collect flags
    ApoSectionFlags flags = 0;
    while (true) {
        Token t = p->current;
        switch (p->current.kind) {
        case TOK_NEWLINE:
            advance(p);
            break;
        case TOK_KW_UNMAPPED:
            flags |= APO_SECFL_UNMAPPED;
            advance(p);
            break;
        case TOK_KW_WRITABLE:
            flags |= APO_SECFL_WRITABLE;
            advance(p);
            break;
        case TOK_KW_EXECUTABLE:
            flags |= APO_SECFL_EXECUTABLE;
            advance(p);
            break;
        case TOK_KW_BLANK:
            flags |= APO_SECFL_BLANK;
            advance(p);
            break;
        case TOK_KW_PINNED:
            flags |= APO_SECFL_PINNED;
            advance(p);
            break;
        case TOK_KW_COMMON:
            flags |= APO_SECFL_COMMON;
            advance(p);
            break;
        case TOK_KW_NONVOLATILE:
            flags |= APO_SECFL_NONVOLATILE;
            advance(p);
            break;
        case TOK_KW_UNIQUE:
            flags |= APO_SECFL_UNIQUE;
            advance(p);
            break;
        case TOK_KW_SECTION:
            parse_section(p, flags);
            return;
        default:
            parse_error(p, p->cursor, "expected a section");
            UNREACHABLE;
        }
    }
}

Parser parser_new(LunaInstance* luna, Vec(Token) tokens) {
    Parser p = {
        .tokens = tokens,
        .cursor = 0,
        .current = tokens[0],
        .sections = vec_new(Section*, 16),
        .current_section_index = 0,
        .current_section = nullptr,
        .luna = luna,
        .symbols = vec_new(Symbol, 256),
        .exprs = vec_new(ComplexExpr, 256),
    };

    // append the null symbol
    vec_append(&p.symbols, ((Symbol){
        .bind = SYM_UNDEFINED,
        .name = nullptr,
        .name_len = 0,
        .section_def = 0,
        .section_offset = 0,
    }));

    strmap_init(&p.symbol_indexes, 256);

    return p;
}

Object parse_tokenbuf(Parser* p) {
    Object obj;
    while (true) {
        switch (p->current.kind) {
        case TOK_EOF:
            obj = (Object){
                .tokens = p->tokens,
                .sections = p->sections,
                .exprs = p->exprs,
                .symbol_indexes = p->symbol_indexes,
                .symbols = p->symbols,
                .luna = p->luna,
            };
            return obj;
        case TOK_NEWLINE:
            advance(p);
            continue;
        case TOK_KW_UNMAPPED:
        case TOK_KW_WRITABLE:
        case TOK_KW_EXECUTABLE:
        case TOK_KW_BLANK:
        case TOK_KW_PINNED:
        case TOK_KW_COMMON:
        case TOK_KW_NONVOLATILE:
        case TOK_KW_UNIQUE:
            parse_section_flags(p);
            break;
        case TOK_KW_SECTION:
            parse_section(p, 0);
            break;
        default:
            parse_error(p, p->cursor, "expected section or declaration");
        }
    }
}
