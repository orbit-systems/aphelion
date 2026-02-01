#include "aphelion.h"
#include "lex.h"
#include "parse.h"
#include "reporting.h"

#include "common/util.h"

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

ExprValue evaluate_expr(const Object* restrict o, u32 expr_index) {
    const ComplexExpr* expr = &o->exprs[expr_index];
    switch (expr->kind) {
    case CEXPR_VALUE:
        return EXPR_CONST(expr->value);
    case CEXPR_SYMBOL_REF: {
        const Symbol* sym = &o->symbols[expr->symbol_ref];
        if (sym->bind == SYM_UNDEFINED) {
            parse_error(o, expr->token_index, "symbol is undefined");
        }
        u64 sym_value = o->sections[sym->section_def]->address + sym->section_offset;
        return EXPR_W_ORIGIN(sym_value, sym->section_def);
    }
    case CEXPR_NEG: {
        ExprValue inner = evaluate_expr(o, expr->un);
        if (inner.origin != ORIGIN_CONSTANT) {
            parse_warn(o, expr->token_index, "expression is not relocatable");
        }
        return EXPR_CONST(-inner.value);
    }
    case CEXPR_NOT:{
        ExprValue inner = evaluate_expr(o, expr->un);
        if (inner.origin != ORIGIN_CONSTANT) {
            parse_warn(o, expr->token_index, "expression is not relocatable");
        }
        return EXPR_CONST(~inner.value);
    }
    case CEXPR_ADD:
    case CEXPR_SUB:
    case CEXPR_MUL:
    case CEXPR_DIV:
    case CEXPR_REM:
    case CEXPR_AND:
    case CEXPR_OR: {
        ExprValue lhs = evaluate_expr(o, expr->bin.lhs);
        ExprValue rhs = evaluate_expr(o, expr->bin.rhs);
        u64 value;

        if (expr->kind != CEXPR_SUB &&
            (lhs.origin != ORIGIN_CONSTANT ||
            lhs.origin != ORIGIN_CONSTANT)) {
            parse_warn(o, expr->token_index, "expression is not relocatable");
        }
        if (expr->kind == CEXPR_SUB &&
            (lhs.origin != rhs.origin)) {
            parse_warn(o, expr->token_index, "expression is not relocatable");
        }

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
        return EXPR_CONST(value);
    }
    }
    UNREACHABLE;
}

static void print_expr(const Object* o, u32 index) {

    const char* opstr = "";

    const ComplexExpr* expr = &o->exprs[index];
    switch (expr->kind) {
    case CEXPR_VALUE:
        printf("%lu", expr->value);
        break;
    case CEXPR_SYMBOL_REF:
        const Symbol* sym = &o->symbols[expr->symbol_ref];
        printf("\"%.*s\"", sym->name_len, sym->name);
        break;
    case CEXPR_NEG:
        printf("-");
        print_expr(o, expr->un);
        break;
    case CEXPR_NOT:
        printf("~");
        print_expr(o, expr->un);
        break;
    case CEXPR_ADD:
        opstr = "+";
        goto print_binop;
    case CEXPR_SUB:
        opstr = "-";
        goto print_binop;
    case CEXPR_MUL:
        opstr = "*";
        goto print_binop;
    case CEXPR_DIV:
        opstr = "/";
        goto print_binop;
    case CEXPR_REM:
        opstr = "%";
        goto print_binop;
    case CEXPR_AND:
        opstr = "&";
        goto print_binop;
    case CEXPR_OR:
        opstr = "|";
        print_binop:
        printf("(");
        print_expr(o, expr->bin.lhs);
        printf(" %s ", opstr);
        print_expr(o, expr->bin.rhs);
        printf(")");
        break;
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
                    printf(" = %lu", value);
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

            // nothing really needs to be done here YET since complex things like
            // LI and CALL aren't handled yet.

            switch ((InstName)elem->kind) {
            case INST_P_CALL:
            case INST_P_LI:
                TODO("handle specialization of call and li");
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
