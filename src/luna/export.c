#include "common/vec.h"
#include "parse.h"
#include "common/util.h"
#include "export.h"

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

static u32 cexpr_to_symbol(ComplexExpr* expr) {
    if (expr->kind != CEXPR_SYMBOL_REF) {
        return 0;
    }
    return expr->symbol_ref;
}

string export_flat_binary(const Object* o) {
    u64 out_len = 0;
    for_n(i, 0, vec_len(o->sections)) {
        Section* section = o->sections[i];
        out_len = max(out_len, section->address + section->bytesize);
    }

    string output;
    output.len = out_len;
    output.raw = malloc(out_len);


    for_n(i, 0, vec_len(o->sections)) {
        Section* section = o->sections[i];

        u64 offset = 0;
        for_n(i, 0, vec_len(section->elements)) {
            
            SectionElement* elem = &section->elements[i];

            // encode elements!
            switch (elem->kind) {
            case ELEM_LABEL:
            case ELEM_EXPR:
            case ELEM_SKIP:
                break;
            default:
                TODO("unimpl %d", elem->kind);
            case ELEM_INST__BEGIN ... ELEM_INST__END:
                // break;
            }
        }
    }

    return output;
}
