#include "aphelion.h"
#include "lex.h"
#include "parse.h"
#include "reporting.h"

#include "common/util.h"
#include "common/portability.h"

#include <stdio.h>

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

static void parse_error__internal(const Object* p, ReportKind severity, const char* begin, string msg) {

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

    free(msg.raw);

    if (severity == REPORT_ERROR) {
        exit(1);
    }
}

static const i8 complex_expr_state[CEXPR__END][2][2] = {
/*   OP              ABS  ---    SYM  ---  */
    [CEXPR_NEG] = { {0, -1}, { 1, -1} },
    [CEXPR_NOT] = { {0, -1}, {-1, -1} },

/*   OP             A/A  A/S  S/A  S/S  */
    [CEXPR_ADD] = { {0,  1}, {1,  -1} },
    [CEXPR_SUB] = { {0, -1}, {1,   0} },
    [CEXPR_MUL] = { {0, -1}, {-1, -1} },
    [CEXPR_DIV] = { {0, -1}, {1,  -1} },
    [CEXPR_REM] = { {0, -1}, {1,  -1} },

    [CEXPR_AND] = { {0, -1}, {-1, -1} },
    [CEXPR_OR]  = { {0, -1}, {-1, -1} },
};

static const char* cexpr_opstr(ComplexExprKind k) {
    switch (k) {
    case CEXPR_SUB:
    case CEXPR_NEG: return "-";
    case CEXPR_NOT: return "~";
    case CEXPR_ADD: return "+";
    case CEXPR_MUL: return "*";
    case CEXPR_DIV: return "/";
    case CEXPR_REM: return "%";
    case CEXPR_AND: return "&";
    case CEXPR_OR:  return "|";
    default:
        return nullptr;
    }
}

ExprValue evaluate_expr(const Object* restrict o, u32 expr_index) {
    const ComplexExpr* expr = &o->exprs[expr_index];
    switch (expr->kind) {
    case CEXPR_VALUE:
        return EXPR_CONST(expr->value);
    case CEXPR_SYMBOL_REF: {
        if (o->symbols[expr->symbol_ref].bind == SYM_UNDEFINED) {
            parse_error(o, expr->token_index, "symbol is undefined");
        }

        return EXPR_SYM(expr->symbol_ref);
    }
    case CEXPR_NEG: FALLTHROUGH;
    case CEXPR_NOT: {
        ExprValue inner = evaluate_expr(o, expr->un);

        i8 dep = *complex_expr_state[expr->kind][EXPR_SYM_DEP(inner.symbol_index)];

        if (dep < 0) {
        // TODO: specialize error. This is only meaningful for neg and not
            parse_warn(o, expr->token_index, "Cannot relocate '%s': does not support symbolic operand", cexpr_opstr(expr->kind));
        dep = 0; // treat as abs, no longer relocatable
        }


    u64 value = inner.value;

    switch (expr->kind) {
        case CEXPR_NEG:
            value = -value;
            break;
        case CEXPR_NOT:
            value = ~value;
            break;
        default:
            UNREACHABLE;
    }

    // No idea what unary op would support a symbol but here for completeness
    // Currently unreachable since both NOT and NEG dont accept symbols
    if (dep == 1) {
        return EXPR_SYM_ADD(value, inner.symbol_index);
    }

    return EXPR_CONST(value);
    }
    case CEXPR_ADD:
    case CEXPR_SUB:
    case CEXPR_MUL:
    case CEXPR_DIV:
    case CEXPR_REM:
    case CEXPR_AND: FALLTHROUGH;
    case CEXPR_OR: {
        ExprValue lhs = evaluate_expr(o, expr->bin.lhs);
        ExprValue rhs = evaluate_expr(o, expr->bin.rhs);

        i8 dep = complex_expr_state[expr->kind][EXPR_SYM_DEP(lhs.symbol_index)][EXPR_SYM_DEP(rhs.symbol_index)];

        // An ABS result is only valid if both operands have the same origin
        // (ABS/ABS or SYM/SYM in the same section)
        char* msg = "";

        u32 lhs_sec = EXPR_SYM_DEP(lhs.symbol_index) ? o->symbols[lhs.symbol_index].section_def : 0;
        u32 rhs_sec = EXPR_SYM_DEP(rhs.symbol_index) ? o->symbols[rhs.symbol_index].section_def : 0;

        // two symbols cancelled to abs, but we cant relocate if they belong
        // to different sections
        if (dep == 0 && lhs_sec != rhs_sec) {
            dep = -1;
            msg = "symbol origins differ";
        }

        if (dep < 0) {
                parse_warn(o, expr->token_index, "Cannot relocate '%s' (%s %s %s): %s",
            cexpr_opstr(expr->kind),
                    EXPR_SYM_DEP(lhs.symbol_index) ? "symbolic" : "constant",
            cexpr_opstr(expr->kind),
                    EXPR_SYM_DEP(rhs.symbol_index) ? "symbolic" : "constant",
               msg
                );
            dep = 0; // treat as abs, no longer relocatable
        }

        u64 value;

        switch (expr->kind) {
        case CEXPR_ADD:
            value = lhs.value + rhs.value;
            break;
        case CEXPR_SUB:
            value = lhs.value - rhs.value;
            break;
        case CEXPR_MUL:
            value = lhs.value * rhs.value;
            break;
        case CEXPR_DIV:
            if (rhs.value == 0) {
                parse_error(o, expr->token_index, "divisor is zero");
            }
            value = lhs.value / rhs.value;
            break;
        case CEXPR_REM:
            if (rhs.value == 0) {
                parse_error(o, expr->token_index, "divisor is zero");
            }
            value = lhs.value % rhs.value;
            break;
        case CEXPR_AND:
            value = lhs.value & rhs.value;
            break;
        case CEXPR_OR:
            value = lhs.value | rhs.value;
            break;
        default:
            UNREACHABLE;
        }

        // symbolic, eligible for relocation
        if (dep == 1) {
            u64 symbol_index = EXPR_SYM_DEP(lhs.symbol_index) ? lhs.symbol_index : rhs.symbol_index;

            i64 addend = int_in_bits(value, 16);

            if (addend != (i64)value) {
                parse_warn(o, expr->token_index,
                    "addend is truncated from %ld (0x%lx) to %d (0x%x)\n"
                    "Hint: relocation addends must fit in an i16",
                    (i64)value,
                    (i64)value,
                    addend,
                    addend
                );
            }

            return EXPR_SYM_ADD(value, symbol_index);
        }

        return EXPR_CONST(value);
    }
    default:
    }
    UNREACHABLE;
}

static void print_expr(const Object* o, u32 index) {
    const ComplexExpr* expr = &o->exprs[index];
    const char* opstr = cexpr_opstr(expr->kind);

    switch (expr->kind) {
    case CEXPR_VALUE:
        printf("%lu", expr->value);
        break;
    case CEXPR_SYMBOL_REF:
        const Symbol* sym = &o->symbols[expr->symbol_ref];
        printf("\"%.*s\"", sym->name_len, sym->name);
        break;
    case CEXPR_NEG:
        printf("%s", opstr);
        print_expr(o, expr->un);
        break;
    case CEXPR_NOT:
        printf("%s", opstr);
        print_expr(o, expr->un);
        break;
    case CEXPR_ADD:
    case CEXPR_SUB:
    case CEXPR_MUL:
    case CEXPR_DIV:
    case CEXPR_REM:
    case CEXPR_AND:
    case CEXPR_OR:
        printf("%s", opstr);
        printf("(");
        print_expr(o, expr->bin.lhs);
        printf(" %s ", opstr);
        print_expr(o, expr->bin.rhs);
        printf(")");
        break;
    default:
        UNREACHABLE;
    }
}

void object_dbgprint(const Object* o) {

    printf("SYMBOLS\n");
    {
        int max_sym_len = 0;
        for_n(i, 1, vec_len(o->symbols)) {
            const Symbol* sym = &o->symbols[i];
            max_sym_len = max(max_sym_len, sym->name_len);
        }

        printf("INDEX  %-*s  BIND    SECTION  OFFSET\n", max_sym_len, "NAME");
        for_n(i, 1, vec_len(o->symbols)) {
            const Symbol* sym = &o->symbols[i];

            printf("%-5lu  %-*.*s  ", i, max_sym_len, sym->name_len, sym->name);

            switch (sym->bind) {
            case SYM_UNDEFINED:
                printf("undef ");
                break;
            case SYM_LOCAL:
                printf("local ");
                break;
            case SYM_GLOBAL:
                printf("global");
                break;
            case SYM_WEAK:
                printf("weak  ");
                break;
            }

            printf("  %-7d  %x\n", sym->section_def, sym->section_offset);
        }
    }
    printf("\nRELOCATIONS\n");
    {
        u32 patch_len = strlen("PATCH");
        u32 sym_len = strlen("SYMBOL");
        for_n(i, 0, vec_len(o->relocs)) {
            patch_len = max(patch_len, o->relocs[i].patch_sec->name_len);
            sym_len = max(sym_len, o->symbols[o->relocs[i].symbol_index].name_len);
        }

        printf("INDEX  %-*s  ELEM   KIND         %-*s  ADDEND\n", patch_len, "PATCH", sym_len, "SYMBOL");

        for_n(i, 0, vec_len(o->relocs)) {
            const Relocation* r = &o->relocs[i];
            const Symbol* sym = &o->symbols[r->symbol_index];

            printf("%-5lu  %-*.*s  %-5d  ", i + 1, patch_len, r->patch_sec->name_len, r->patch_sec->name, r->patch_elem_index);
            switch(r->kind) {
                case RELOC_WORD:
                    printf("Word         ");
                    break;
                case RELOC_WORD_UNALIGNED:
                    printf("Word U.Align.");
                    break;
                case RELOC_CALL:
                    printf("Call         ");
                    break;
                case RELOC_FARCALL:
                    printf("Far call     ");
                    break;
                case RELOC_LI:
                    printf("Load Imm.    ");
                    break;
                case RELOC_BRANCH:
                    printf("Branch       ");
                    break;
                default:
                    TODO("Implement");
            }
            printf("%-*.*s  %-4x\n", sym_len, sym->name_len, sym->name, r->addend);
        }
    }

    printf("\nSECTIONS\n");
    {
        for_n(i, 0, vec_len(o->sections)) {
            const Section* sec = o->sections[i];

            printf("%lu \"%.*s\" ",
                i,
                sec->name_len, sec->name
            );

            static const char* const flags[] = {
                "unmapped",
                "writable",
                "executable",
                "threadlocal",
                "blank",
                "pinned",
                "common",
                "nonvolatile",
                "concatenate",
            };

            for_n(i, 0, sizeof(flags) / sizeof(flags[0])) {
                if (sec->flags & (1 << i)) {
                    printf("%s ", flags[i]);
                }
            }

            printf("0x%lx ", sec->address);

            printf("\n");

            for_n(i, 0, vec_len(sec->elements)) {
                printf("  ");
                const SectionElement* elem = &sec->elements[i];
                switch (elem->kind) {
                case ELEM_INST__BEGIN ... ELEM_INST__END:
                    printf("%-5s %s %s %s %d",
                        inst_name_string[elem->inst.name],
                        gpr_name[elem->inst.r1],
                        gpr_name[elem->inst.r2],
                        gpr_name[elem->inst.r3],
                        elem->inst.imm
                    );
                    break;
                case ELEM_LABEL: {
                    const Symbol* sym = &o->symbols[elem->label.symbol_index];
                    printf("label %u \"%.*s\"", elem->label.symbol_index, sym->name_len, sym->name);
                } break;
                case ELEM_EXPR: {
                    u64 value = evaluate_expr(o, elem->expr.index).value;
                    printf("\\ expr ");
                    print_expr(o, elem->expr.index);
                    printf(" = %lu (0x%016lx) ", value, value);
                } break;
                case ELEM_STRING: {
                    printf("string\n");
                    int len = elem->string.length;
                    i += 1;
                    elem = &sec->elements[i];
                    printf("  ^- \"%.*s\"", len, elem->supp_string);
                } break;
                case ELEM_SKIP:
                    break;
                default:
                    printf("unknown %d", elem->kind);
                }
                printf("\n");
            }
        }
    }
}

// assume align is a power of two
static uintptr_t align_forward(uintptr_t ptr, uintptr_t align) {
    return (ptr + align - 1) & ~(align - 1);
}

static u64 inst_size(InstName instname) {
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

/// Returns new address to start placing the next section at.
static u64 section_trace(Object* o, Section* section, u64 section_index, u64 starting_pos) {
    u64 section_address = align_forward(starting_pos, 1ull << section->alignment_p2);

    if (section->address_specified) {
        section_address = section->address;
    } else {
        section->address = section_address;
    }

    u64 address = section_address;

    for_n(i, 0, vec_len(section->elements)) {
        SectionElement elem = section->elements[i];
        u32 token_index = section->elem_tokens[i];

        switch (elem.kind) {
        case ELEM_INST__BEGIN ... ELEM_INST__END:
            if (address % 4 != 0) {
                parse_error(o, token_index, "instruction is not effectively aligned");
                // CRASH("instruction is not effectively aligned");
            }
            address += inst_size(elem.inst.name);
            break;
        case ELEM_LABEL: {
            Symbol* sym = &o->symbols[elem.label.symbol_index];
            sym->section_def = section_index;
            sym->section_offset = address - section_address;
        } break;
        case ELEM_STRING: {
            address += elem.string.length;
            i += 1; // skip past supplemental data
        } break;
        case ELEM_SKIP:
        case ELEM_EXPR:
            break;
        default:
            TODO("GUH %d", elem.kind);
        }
    }

    section->bytesize = address - section_address;

    return address;
}

static ComplexExpr* cexpr(Object* o, u32 index) {
    return &o->exprs[index];
}

static void specialize_const_li(Object *o, Section* sec, SectionElement* start, u64 imm) {
    u8 reg = start->inst.r1;

    printf("0x%x ", imm);
    bool sign = imm & 0x8000000000000000UL;

    // quarter mask: Q0 = lowest 16 bits, Q3 = highest
    u8 quarter = 0;
    for_n(i, 0, 4) {
        SectionElement* elem = start + i;
        elem->kind = ELEM_SKIP; // We saved what we needed.
        if ((imm >> (i * 16)) & 0xFFFF)
            quarter |= 1u << i;
    }

    for_n_reverse(i, 3, 0) {
        SectionElement* elem = start + i;
        u16 chunk = (imm >> (i * 16)) & 0xFFFF;

        // evil bit magic: dont clear if theres data above
        // (since may we have already sign extended+cleared).
        u8 higher = quarter & ~((1u << (i + 1)) - 1);

        // emit if quarter has data or it's the last quarter and imm is zero
        if (!(quarter & (1u << i)) && !(i == 0 && quarter == 0)) continue;



        elem->inst.imm = (chunk << 3) | ((higher != 0) << 2) | i;
        elem->inst.name = (higher != 0) ? INST_SSI : INST_SSI_C;
        elem->inst.r1 = reg;

        // Q4 special logic: always clear below it
        if (i == 3) {
        elem->inst.name = INST_SSI_C;
        elem->inst.imm |= sign << 2;
        }
    }
}

static void specialize_li(Object*o, Section* section, SectionElement* start, u64 index) {
    // Guaranteed to have 4 elems after it
    SectionElement* li   = start;
    SectionElement* expr = start + 1;

    ExprValue v = evaluate_expr(o, expr->expr.index);

    if (EXPR_SYM_DEP(v.symbol_index)) {
        Relocation r = {
            .kind = RELOC_LI,
            .patch_sec = section,
            .patch_elem_index = index,
            .symbol_index = v.symbol_index,
            .addend = (i16) v.value,
        };
        vec_append(&o->relocs, r);
    } else {
        specialize_const_li(o, section, start, v.value);
    }
}

void object_trace(Object* o) {
    u64 address = 0;
    for_n(i, 0, vec_len(o->sections)) {
        Section* section = o->sections[i];
        address = section_trace(o, section, i, address);
    }

    // specialize instructions based on the information
    // obtained by the previous pessimistic trace
    for_n(i, 0, vec_len(o->sections)) {
        Section* section = o->sections[i];
        for_n(i, 0, vec_len(section->elements)) {
            SectionElement* elem = &section->elements[i];

            switch ((InstName)elem->kind) {
            case INST_P_CALL:
                TODO("handle specialization of call");
            case INST_P_LI:
                specialize_li(o, section, elem, i);
                break;
            default:
                break;
            }
        }
    }

    address = 0;
    for_n(i, 0, vec_len(o->sections)) {
        Section* section = o->sections[i];
        address = section_trace(o, section, i, address);
    }
}
