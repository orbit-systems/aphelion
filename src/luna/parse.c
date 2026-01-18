#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdckdint.h>

#include "parse.h"
#include "aphelion.h"
#include "common/strmap.h"
#include "common/util.h"
#include "common/vec.h"
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


static u32 encode_fmt_a(AphelOpcode op, AphelGpr r1, u32 imm19) {
    u32 bits = 0;
    bits |= (u32)op;
    bits |= r1 << 8;
    bits |= imm19 << 13;
    return bits;
}

static u32 encode_fmt_b(AphelOpcode op, AphelGpr r1, AphelGpr r2, u32 imm14) {
    u32 bits = 0;
    bits |= op;
    bits |= r1 << 8;
    bits |= r2 << 13;
    bits |= imm14 << 18;
    return bits;
}

static u32 encode_fmt_c(AphelOpcode op, AphelGpr r1, AphelGpr r2, AphelGpr r3, u32 imm9) {
    u32 bits = 0;
    bits |= op;
    bits |= r1 << 8;
    bits |= r2 << 13;
    bits |= r3 << 18;
    bits |= imm9 << 23;
    return bits;
}

const bool inst_name_imm_signed[INST__COUNT] = {
    [INST_SSI_C] = true,
    [INST_MUL] = true,
    [INST_IMULH] = true,
    [INST_IDIV] = true,
    [INST_MULI] = true,
    [INST_IREM] = true,
    [INST_IDIVI] = true,
    [INST_IREMI] = true,
    [INST_SEQI] = true,
    [INST_SILTI] = true,
    [INST_SILEI] = true,
    [INST_BZ] = true,
    [INST_BN] = true,
    [INST_JL] = true,
    [INST_JLR] = true,
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

    [INST_FENCE_S ... INST_FENCE_SL] = OP_FENCE,

    [INST_CINVAL_BLOCK ... INST_CINVAL_D_ALL] = OP_CINVAL,

    [INST_CFETCH_S ... INST_CFETCH_SLI] = OP_CFETCH,

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

static void parse_instruction(Parser* p);
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
        parse_error(p, p->cursor, "expected %s, got %s", 
            token_kind_name[kind],
            token_kind_name[p->current.kind]
        );
    }
}

static inline void expect_advance(Parser* p, TokenKind kind) {
    expect_kind(p, kind);
    advance(p);
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

typedef struct TokenSpan {
    usize start;
    usize end;
} TokenSpan;

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

[[gnu::const]]
static ComplexExpr* cexpr(Parser* p, u32 index) {
    return &p->exprs[index];
}

[[gnu::const]]
static i64 int_in_bits(i64 n, usize bits) {
    return ((n << (64 - bits)) >> (64 - bits));
}

[[gnu::const]]
static u64 uint_in_bits(u64 n, usize bits) {
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

    usize ident_err_index = p->cursor;
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
        parse_error(p, ident_err_index, "symbol too long (max %d)", UINT16_MAX);
    }

    usize sym_index = symbol_find_or_create_undef(p, ident);
    Symbol* sym = &p->symbols[sym_index];
    if (sym->bind != SYM_UNDEFINED) {
        parse_error(p, ident_err_index, "symbol is already defined", UINT16_MAX);
    }
    sym->bind = bind;
    sym->section_def = p->current_section_index;

    SectionElement label = { .label = {
        ._kind = ELEM_LABEL,
        .symbol_index = sym_index,
    }};

    vec_append(&p->current_section->elements, label);
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

    vec_append(&p->current_section->elements, align);
}

static void parse_directive_string(Parser* p) {
    expect_advance(p, TOK_KW_STRING);
    string contents = parse_strlit_contents(p);
    expect_advance(p, TOK_NEWLINE);

    SectionElement string_1 = { .string = {
        ._kind = ELEM_STRING,
        .length = contents.len,
    }};
    SectionElement string_2 = {
        .supp_string = contents.raw,
    };

    vec_append(&p->current_section->elements, string_1);
    vec_append(&p->current_section->elements, string_2);
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
//         // imm_expr_elem.kind = ELEM_IMM_EXPR;
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
        cexpr(p, index)->kind = token_index;
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
        cexpr(p, index)->kind = token_index;
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

/// An operand representing a memory location, like `[r1 + r2 + imm]`
typedef struct MemOperand {
    AphelGpr r1;
    AphelGpr r2;
    usize addend;
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

    expect_kind(p, TOK_GPR);
    mem.r1 = p->current.subkind;

    if (p->current.kind == TOK_PLUS) {
        advance(p);
        if (p->current.kind == TOK_GPR) {
            if (no_r2) {
                parse_error(p, p->cursor, "this operand only accepts a single register");
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

static AphelGpr parse_operand_gpr(Parser* p) {
    AphelGpr reg = p->current.subkind;
    expect_advance(p, TOK_GPR);
    return reg;
}

static void parse_arith_rr(Parser* p) {
    Token inst = p->current;

    expect_advance(p, TOK_INST);

    AphelGpr r1 = parse_operand_gpr(p);
    expect_advance(p, TOK_COMMA);
    AphelGpr r2 = parse_operand_gpr(p);
    expect_advance(p, TOK_COMMA);
    AphelGpr r3 = parse_operand_gpr(p);

    i64 imm = 0;
    SectionElement imm_expr_elem = {0};
    if (p->current.kind == TOK_COMMA) {
        advance(p);
        usize err_token = p->cursor;
        u32 imm_expr = parse_expr(p);

        if (cexpr(p, imm_expr)->kind == CEXPR_VALUE) {
            // extract the constant value and encode it directly
            imm = cexpr(p, imm_expr)->value;
            if (inst_name_imm_signed[inst.subkind]) {
                if (imm != int_in_bits(imm, 9)) {
                    parse_warn(p, err_token, 
                        "constant expression is truncated to %d", 
                        int_in_bits(imm, 9)
                    );
                }
            } else {
                if (imm != uint_in_bits(imm, 9)) {
                    parse_warn(p, err_token, 
                        "constant expression is truncated to %u", 
                        uint_in_bits(imm, 9)
                    );
                }
            }
        } else {
            imm_expr_elem.kind = ELEM_IMM_EXPR;
            imm_expr_elem.expr.index = imm_expr;
        }
    }
    expect_advance(p, TOK_NEWLINE);

    SectionElement elem = {.inst = {
        .name = inst.subkind,
        .r1 = r1,
        .r2 = r2,
        .r3 = r3,
        .imm = imm,
    }};
    vec_append(&p->current_section->elements, elem);

    if (imm_expr_elem.kind != 0) {
        vec_append(&p->current_section->elements, imm_expr_elem);
    }
}

static void parse_branch(Parser* p) {
    Token inst = p->current;

    expect_advance(p, TOK_INST);
    
    WRAP_SPAN(span, AphelGpr r1 = parse_operand_gpr(p));
    expect_advance(p, TOK_COMMA);

    u32 imm_expr = parse_expr(p);
    i64 imm = 0;

    SectionElement imm_expr_elem = {0};
    if (cexpr(p, imm_expr)->kind == CEXPR_VALUE) {
        // extract the constant value and encode it directly
        imm = cexpr(p, imm_expr)->value;
        if (imm != int_in_bits(imm, 19)) {
            parse_warn(p, span.start, 
                "constant expression is truncated to %d", 
                int_in_bits(imm, 19)
            );
        }
    } else {
        imm_expr_elem.kind = ELEM_IMM_EXPR;
        imm_expr_elem.expr.index = imm_expr;
    }

    expect_advance(p, TOK_NEWLINE);

    SectionElement elem = {.inst = {
        .name = inst.subkind,
        .r1 = r1,
        .imm = imm,
    }};
    vec_append(&p->current_section->elements, elem);

    if (imm_expr_elem.kind != 0) {
        vec_append(&p->current_section->elements, imm_expr_elem);
    }
}

static void parse_instruction(Parser* p) {
    Token inst = p->current;

    switch (inst.subkind) {
    case INST_ADD ... INST_XOR:
        // inst gpr, gpr, gpr [, imm]
        parse_arith_rr(p);
        break;
    case INST_BZ:
    case INST_BN:
        parse_branch(p);
        break;
    default:
        parse_error(p, p->cursor, "expected instruction");
    }
}

static void parse_section(Parser* p, ApoSectionFlags flags) {
    Arena* arena = &p->luna->permanent;

    expect_advance(p, TOK_KW_SECTION);
    string section_name = parse_strlit_contents(p);

    u64 alignment = 8;

    while (true) {
        if (p->current.kind == TOK_KW_ALIGN) {
            // section-level align
            advance(p);
            usize index = p->cursor;
            alignment = parse_const_expr(p);
            if (!is_pow_2(alignment)) {
                parse_error(p, index, "section align is not a power of two");
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
        .elements = vec_new(SectionElement, 64),
    };

    if (p->current_section != nullptr) {
        p->current_section_index += 1;
    }

    vec_append(&p->sections, section);
    p->current_section = section;

    while (parse_sec_element(p)) {}
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
        case TOK_KW_CONCATENATE:
            flags |= APO_SECFL_CONCATENATE;
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
        case TOK_KW_CONCATENATE:
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
