#include "aphelion.h"
#include "apollo/apollo.h"
#include "common/str.h"
#include "lex.h"
#include "parse.h"
#include "common/util.h"
#include <stdio.h>

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

void object_debug(const Object* o) {

    printf("SYMBOLS\n");
    {
        int max_sym_len = 0;
        for_n(i, 1, vec_len(o->symbols)) {
            const Symbol* sym = &o->symbols[i];
            max_sym_len = max(max_sym_len, sym->name_len);
        }

        printf("INDEX  %-*s  BIND    SECTION  OFFSET\n", max_sym_len, "NAME");
        for_n(i, 0, vec_len(o->symbols)) {
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
                "group.header",
                "group.member",
            };
            
            for_n(i, 0, sizeof(flags) / sizeof(flags[0])) {
                if (sec->flags & (1 << i)) {
                    printf("%s ", flags[i]);
                }
            }

            if (sec->address_specified) {
                printf("%lx ", sec->address);
            }

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
                case ELEM_IMM_EXPR:
                    printf("-> expr ");
                    print_expr(o, elem->expr.index);
                    break;
                case ELEM_STRING: {
                    printf("string\n");
                    int len = elem->string.length;
                    i += 1;
                    elem = &sec->elements[i];
                    printf("  -> \"%.*s\"", len, elem->supp_string);
                } break;
                default:
                    printf("unknown %d", elem->kind);
                }
                printf("\n");
            }
            // printf("\n");
        }
    }
}

static u64 section_trace_pessimistic(Section* section, u64 starting_pos) {
    
}

void object_trace_pessimistic(Object* o) {
    u64 address = 0;
    for_n(i, 0, vec_len(o->sections)) {
        Section* section = o->sections[i];
        
    }
}
