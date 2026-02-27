#include "export.h"

#include "aphelion.h"
#include "lex.h"
#include "parse.h"

#include "common/portability.h"
#include "common/util.h"
#include "common/vec.h"

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

static AphelOpcode instname_opcode[INST__COUNT] = {
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

    [INST_ADD]   = OP_ADD,
    [INST_SUB]   = OP_SUB,
    [INST_MUL]   = OP_MUL,
    [INST_UMULH] = OP_UMULH,
    [INST_IMULH] = OP_IMULH,
    [INST_UDIV]  = OP_UDIV,
    [INST_IDIV]  = OP_IDIV,
    [INST_UREM]  = OP_UREM,
    [INST_IREM]  = OP_IREM,

    [INST_AND] = OP_AND,
    [INST_OR]  = OP_OR,
    [INST_NOR] = OP_NOR,
    [INST_XOR] = OP_XOR,

    [INST_ADDI]  = OP_ADDI,
    [INST_SUBI]  = OP_SUBI,
    [INST_MULI]  = OP_MULI,
    [INST_UDIVI] = OP_UDIVI,
    [INST_IDIVI] = OP_IDIVI,
    [INST_UREMI] = OP_UREMI,
    [INST_IREMI] = OP_IREMI,

    [INST_ANDI] = OP_ANDI,
    [INST_ORI]  = OP_ORI,
    [INST_NORI] = OP_NORI,
    [INST_XORI] = OP_XORI,

    [INST_SL]  = OP_SL,
    [INST_USR] = OP_USR,
    [INST_ISR] = OP_ISR,
    [INST_SI_U ... INST_SI_I] = OP_SI,
    [INST_CB]  = OP_CB,
    [INST_ROR] = OP_ROR,
    [INST_ROL] = OP_ROL,

    [INST_REV ... INST_REV_BIT] = OP_REV,

    [INST_CSB] = OP_CSB,
    [INST_CLZ] = OP_CLZ,
    [INST_CTZ] = OP_CTZ,
    [INST_EXT] = OP_EXT,
    [INST_DEP] = OP_DEP,

    [INST_SEQ]  = OP_SEQ,
    [INST_SULT] = OP_SULT,
    [INST_SILT] = OP_SILT,
    [INST_SULE] = OP_SULE,
    [INST_SILE] = OP_SILE,

    [INST_SEQI]  = OP_SEQI,
    [INST_SULTI] = OP_SULTI,
    [INST_SILTI] = OP_SILTI,
    [INST_SULEI] = OP_SULEI,
    [INST_SILEI] = OP_SILEI,

    [INST_BZ] = OP_BZ,
    [INST_BN] = OP_BN,

    [INST_JL]  = OP_JL,
    [INST_JLR] = OP_JLR,

    [INST_SYSCALL] = OP_SYSCALL,
    [INST_BREAKPT] = OP_BREAKPT,
    [INST_WAIT]    = OP_WAIT,
    [INST_SPIN]    = OP_SPIN,
    [INST_IRET]    = OP_IRET,
    [INST_LCTRL]   = OP_LCTRL,
    [INST_SCTRL]   = OP_SCTRL,
};

static i64 handle_expr(const Object* o, InstName instname, u64 addr, u32 expr_index) {
    i64 value = evaluate_expr(o, expr_index).value;

    switch (instname) {
    case INST_BZ:
    case INST_BN:
        return (value - addr - 4) >> 2;
    default:
        return value;
    }
}

static u32 encode_inst_elem(const Object* o, u64 addr, SectionElement inst, SectionElement next) {
    AphelOpcode opcode = instname_opcode[inst.inst.name];

    if (opcode == 0) {
        return 0;
    }

    u32 imm = inst.inst.imm;

    if (next.kind == ELEM_EXPR) {
        imm = handle_expr(o, inst.inst.name, addr, next.expr.index);
    }

    u32 data;

    AphelFmt fmt = fmt_from_op(opcode);
    switch (fmt) {
    case FMT_A:
        data = encode_fmt_a(
            opcode,
            inst.inst.r1,
            imm
        );
        break;
    case FMT_B:
        data = encode_fmt_b(
            opcode,
            inst.inst.r1,
            inst.inst.r2,
            imm
        );
        break;
    case FMT_C:
        data = encode_fmt_c(
            opcode,
            inst.inst.r1,
            inst.inst.r2,
            inst.inst.r3,
            imm
        );
        break;
    }

    return data;
}

static u8 write_relocation_patches(u32* patches, Relocation* r, Section* sec, Symbol *sym, u64 addr) {
	// S+A (others may -P in the switch)
	i64 value = (i64)r->addend + sec->address + sym->section_offset;

	switch (r->kind) {
	case RELOC_CALL:
		// S + A - P
		// patch SSI and JL
		value -= (i64)addr;
		patches[0] = ((value >> 2)  & 0xFFFF) << 18;
		patches[1] = ((value >> 16) & 0xFFFF) << 16;
		return 2;
	case RELOC_WORD:
		// S + A
		patches[0] = value & 0x0000'0000'FFFF'FFFF;
		patches[1] = (value & 0xFFFF'FFFF'0000'0000) >> 32;
		return 2;
	case RELOC_LI:
		// S + A
		// patch 4 SSIs.
		for_n(j, 0, 4) {
		    patches[j] = ((value >> (j * 16)) & 0xFFFF) << 16;
		}
		return 4;
	case RELOC_FARCALL:
		// S + A
		// patch 4 SSIs
		for_n(j, 0, 4) {
		    patches[j] = ((value >> (j * 16)) & 0xFFFF) << 16;
		}
		// make last SSI a JL
		patches[0] <<= 2;
		return 4;
	default:
		TODO("Implement relocation");
	}
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
    memset(output.raw, 0, output.len);

    for_n(i, 0, vec_len(o->sections)) {
        Section* section = o->sections[i];
        u32 section_index = i;

        u64 offset = 0;

        Relocation* r = nullptr;
        u32 patches[4] = {0};
        u8 patch_count = 0;
        u8 patch_index = 0;

        for_n(i, 0, vec_len(section->elements)) {
            SectionElement* elem = &section->elements[i];
            u64 addr = section->address + offset;
            u8* to_write = (u8*)&output.raw[addr];

            if (r == nullptr) {
                for_n(j, 0, vec_len(section->relocs)) {
                    Relocation* cur = &section->relocs[j];
                    if (cur->elem_index == i) {
                        r = cur;
                        Symbol* sym = &o->symbols[r->symbol_index];
                        Section* sec = o->sections[sym->section_def];

                        patch_count = write_relocation_patches(patches, r, sec, sym, addr);
			patch_index = 0;
                    }
                }
            }

            // encode elements!
            switch (elem->kind) {
            case ELEM_LABEL:
            case ELEM_EXPR:
            case ELEM_SKIP:
                break;
            case ELEM_STRING: {
                // next element is a string pointer
                usize len = elem->string.length;
                SectionElement* data = &section->elements[i + 1];
                memcpy(to_write, data->supp_string, len);
                offset += len;
                i += 1;
                break;
            }
            case ELEM_INST__BEGIN ... ELEM_INST__END: {
                switch ((InstName)elem->inst.name) {
                case INST_P_FCALL:
                case INST_P_LI:
                case INST_P_CALL:
                    SectionElement* expr = &section->elements[i + 1];
                    ExprValue v = evaluate_expr(o, expr->expr.index);

                    u8 size = elem->pseudo_inst.size / 4;

                    bool is_li = (elem->pseudo_inst.name == INST_P_LI);
                    u8 reg = is_li ? elem->pseudo_inst.r1 : elem->pseudo_inst.r2;

                    bool sign = (bool) (v.value & 0x8000000000000000UL);
                    u8 quarter = 0;

                    SectionElement ssi = { .inst = {
                        .name = INST_SSI,
                        .r1 = reg,
                        .imm = 0,
                    }};

                    for_n_reverse(q, size, 0) {
                        u16 chunk = (v.value >> (q * 16)) & 0xFFFF;

                        // evil bit magic: dont clear if theres data above
                        // (since may we have already sign extended+cleared).
                        u8 higher = quarter & ~((1u << (q + 1)) - 1);

                        // emit if quarter has data or it's the last quarter and imm is zero
                        if (r == nullptr && !(quarter & (1u << i)) && !(q == 0 && quarter == 0)) {
                            continue;
                        }

                        ssi.inst.imm = (chunk << 3) | ((higher != 0) << 2)  | (is_li ? q : 0);
                        ssi.inst.name = (higher != 0) ? INST_SSI : INST_SSI_C;
                        ssi.inst.r1 = reg;

                        // Q4 special logic: always clear below it
                        if (q == 3) {
                            ssi.inst.imm |= sign << 2;
                            ssi.inst.name = INST_SSI_C;
                        }

                        if (q == 0 && !is_li) {
                            ssi.inst.name = INST_JL;
                            ssi.inst.imm = elem->inst.imm << 2;
                        }

                        u32 data = encode_inst_elem(o, addr, ssi, (SectionElement){.kind = ELEM_SKIP});

                        if (r != nullptr)
                            data |= patches[q];

                        *(u32*)to_write = data;
                        to_write += 4;
                        addr += 4;
                        offset += 4;
                }
                if (r != nullptr) {
                    Symbol* sym = &o->symbols[r->symbol_index];
                    Section* sec = o->sections[sym->section_def];
                    const char* reloc_name_string[] = {
                        "S + A     (Word)          ",
                        "S + A     (Word Unaligned)",
                        "S + A - P (Call)          ",
                        "S + A     (Far Call)      ",
                        "S + A     (Load Immediate)"
                    };

                    printf("PATCH: %s -> %.*s(0x%lx) + %.*s(0x%x) + 0x%ld (%lx) %s\n",
                       reloc_name_string[r->kind],
                       sec->name_len, sec->name, sec->address,
                       sym->name_len, sym->name, sym->section_offset,
                       (i64)r->addend, (i64) r->addend,
                       r->kind == RELOC_CALL ? " - P" : ""
                    );
                    r = nullptr;
                }
                    break;
                default: {
                    u32 data = encode_inst_elem(o, addr, *elem, section->elements[i + 1]);
                    *(u32*)to_write = data;
                    offset += 4;
                    break;
                }
                }
                break;
            }
            default:
                TODO("unimpl %d", elem->kind);
            }
        }
    }
    return output;
}
