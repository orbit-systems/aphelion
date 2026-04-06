#include "common/fs.h"
#include "common/str.h"
#include "common/util.h"
#include "common/ansi.h"
#include "../luna/aphelion.h"
#include <stdio.h>

#include "../luna/aphelion.c"

typedef union EncodedInst {
    u8 opcode;
    struct {
        u32 opcode : 8;
        u32 r1 : 5;
        u32 imm : 19;
    } A;
    struct {
        u32 opcode : 8;
        u32 r1 : 5;
        u32 r2 : 5;
        u32 imm : 14;
    } B;
    struct {
        u32 opcode : 8;
        u32 r1 : 5;
        u32 r2 : 5;
        u32 r3 : 5;
        u32 imm : 9;
    } C;
} EncodedInst;

void print_line_start(usize location, string bytes) {
    printf("%05zx:   ", location);

    if (op_name[(u8)bytes.raw[0]] && bytes.len >= 4) {
        EncodedInst einst = *(EncodedInst*)bytes.raw;
        AphelFmt fmt = fmt_from_op(einst.opcode);
    
        switch (fmt) {
        case FMT_A:
            printf(Cyan"%09b ", (einst.A.imm >> 10) & ((1 << 9) - 1));
            printf("%05b ", (einst.A.imm >> 5) & ((1 << 5) - 1));
            printf("%05b ", (einst.A.imm) & ((1 << 5) - 1));
            printf(Red"%05b ", einst.A.r1);
            break;
        case FMT_B:
            printf(Cyan"%09b ", (einst.B.imm >> 5) & ((1 << 9) - 1));
            printf("%05b ", (einst.B.imm) & ((1 << 5) - 1));
            printf(Red"%05b ", einst.B.r2);
            printf("%05b ", einst.B.r1);
            break;
        case FMT_C:
            printf(Cyan "%09b ", einst.C.imm);
            printf(Red "%05b ", einst.C.r3);
            printf("%05b ", einst.C.r2);
            printf("%05b ", einst.C.r1);
            break;
        }

        printf(Yellow"%06b %02b"Reset, einst.opcode >> 2, einst.opcode & 0b11);
        return;
    }

    for_n(i, 0, 4) {
        if (i < bytes.len) {
            printf("%02x ", bytes.raw[i] & 0xFF);
        } else {
            printf("   ");
        }
    }
    printf("                         ");
}

const char* select_inst_name(EncodedInst einst) {
    const char* inst_name = op_name[einst.opcode];

    // get correct inst name, with modifiers
    switch (einst.opcode) {
    case OP_SSI:
        if (einst.A.imm & 1) {
            return "ssi.c";
        }
        break;
    case OP_SI:
        if (einst.B.imm & (1 << 12)) {
            return "si.i";
        } else {
            return "si.u";
        }
    case OP_REV:
        switch (einst.B.imm & 0b111111) {
        case 0b100000: return "rev.h";
        case 0b110000: return "rev.q";
        case 0b111000: return "rev.b";
        case 0b111111: return "rev.bit";
        }
        break;
    case OP_FENCE:
        switch (einst.A.imm & 0b11) {
        case 0b11: return "fence";
        case 0b01: return "fence.l";
        case 0b10: return "fence.s";
        case 0b00: return "fence.<none>";
        }
        break;
    case OP_CINVAL:
        switch (einst.A.imm & 0b1111) {
        case 0b0000: return "cinval.<none>.block";
        case 0b0100: return "cinval.<none>.page";
        case 0b1000: return "cinval.<none>.all";
        case 0b1100: return "cinval.<none>.<invalid>";
        case 0b0001: return "cinval.d.block";
        case 0b0101: return "cinval.d.page";
        case 0b1001: return "cinval.d.all";
        case 0b1101: return "cinval.d.<invalid>";
        case 0b0010: return "cinval.i.block";
        case 0b0110: return "cinval.i.page";
        case 0b1010: return "cinval.i.all";
        case 0b1110: return "cinval.i.<invalid>";
        case 0b0011: return "cinval.block";
        case 0b0111: return "cinval.page";
        case 0b1011: return "cinval.all";
        case 0b1111: return "cinval.<invalid>";
        }
        break;
    case OP_CFETCH:
        switch (einst.A.imm & 0b111) {
        case 0b000: return "cfetch.<none>";
        case 0b001: return "cfetch.l";
        case 0b010: return "cfetch.s";
        case 0b100: return "cfetch.i";
        case 0b011: return "cfetch.ls";
        case 0b110: return "cfetch.si";
        case 0b111: return "cfetch.lsi";
        }
        break;
    case OP_OR:
        if (einst.C.r3 != GPR_ZR && einst.C.imm == 0)
            break;
        if (einst.C.r1 == GPR_ZR && einst.C.r2 == GPR_ZR) {
            return "nop";
        } else {
            return "mov";
        }
    case OP_JL:
        if (einst.B.r1 == GPR_ZR && einst.B.imm == 0) {
            return "ret";
        }
        break;
    }
    return inst_name;
}

u8 mem_width(AphelOpcode opcode) {
    switch (opcode) {
    case OP_LW:
    case OP_SW:
    case OP_LLW:
    case OP_SCW:
        return 8;
    case OP_LH:
    case OP_SH:
    case OP_LLH:
    case OP_SCH:
        return 4;
    case OP_LQ:
    case OP_SQ:
    case OP_LLQ:
    case OP_SCQ:
        return 2;
    case OP_LB:
    case OP_SB:
    case OP_LLB:
    case OP_SCB:
        return 1;
    default:
        UNREACHABLE;
    }
}

void print_arguments(EncodedInst einst, usize location) {
    switch (einst.opcode) {
    case OP_LW:
    case OP_LH:
    case OP_LQ:
    case OP_LB:
    case OP_LLW:
    case OP_LLH:
    case OP_LLQ:
    case OP_LLB:
        printf("%s, [%s", 
            gpr_name[einst.C.r1],
            gpr_name[einst.C.r2]
        );
        if (einst.C.r3) {
            printf(" + %s", gpr_name[einst.C.r3]);
        }
        if (einst.C.imm) {
            printf(" + %u", einst.C.imm * mem_width(einst.opcode));
        }
        printf("]");
        break;
    case OP_SW:
    case OP_SH:
    case OP_SQ:
    case OP_SB:
        printf("[%s", 
            gpr_name[einst.C.r2]
        );
        if (einst.C.r3) {
            printf(" + %s", gpr_name[einst.C.r3]);
        }
        if (einst.C.imm) {
            printf(" + %u", einst.C.imm * mem_width(einst.opcode));
        }
        printf("], %s", gpr_name[einst.C.r1]);
        break;
    case OP_SCW:
    case OP_SCH:
    case OP_SCQ:
    case OP_SCB:
        printf("%s, [%s", 
            gpr_name[einst.C.r2],
            gpr_name[einst.C.r3]
        );
        if (einst.C.imm) {
            printf(" + %u", einst.C.imm * mem_width(einst.opcode));
        }
        printf("], %s", gpr_name[einst.C.r1]);
        break;
    case OP_CINVAL:
        if (((einst.A.imm >> 2) & 0b11) != 2) {
            printf("%s",
                gpr_name[einst.A.r1]
            );
        }
        break;
    case OP_CFETCH:
        printf("%s",
            gpr_name[einst.A.r1]
        );
        break;
    case OP_SSI:
        printf("%s, %u, %u", 
            gpr_name[einst.A.r1], 
            (einst.A.imm >> 3) & 0xFFFF,
            ((einst.A.imm >> 1) & 0b11) * 16
        );
        break;
    case OP_OR:
        if (einst.C.r3 == GPR_ZR && einst.C.imm == 0) {
            if (einst.C.r1 == GPR_ZR && einst.C.r2 == GPR_ZR) {
                break; // nop
            } else {
                printf("%s, %s", gpr_name[einst.C.r1], gpr_name[einst.C.r2]);
                break; // mov
            }
        }
        FALLTHROUGH;
    case OP_ADD:
    case OP_SUB:
    case OP_UMULH:
    case OP_UDIV:
    case OP_UREM:
    case OP_AND:
    case OP_NOR:
    case OP_XOR:
    case OP_SULT:
    case OP_SULE:
        printf("%s, %s, %s",
            gpr_name[einst.C.r1],
            gpr_name[einst.C.r2],
            gpr_name[einst.C.r3]
        );
        if (einst.C.imm != 0) {
            printf(", %d", einst.C.imm);
        }
        break;
    case OP_MUL:
    case OP_IMULH:
    case OP_IDIV:
    case OP_IREM:
    case OP_SEQ:
    case OP_SILT:
    case OP_SILE:
        printf("%s, %s, %s",
            gpr_name[einst.C.r1],
            gpr_name[einst.C.r2],
            gpr_name[einst.C.r3]
        );
        if (einst.C.imm != 0) {
            printf(", %d", ((i32)einst.C.imm << (32-9)) >> (32-9));
        }
        break;
    case OP_ADDI:
    case OP_SUBI:
    case OP_UDIVI:
    case OP_UREMI:
    case OP_ANDI:
    case OP_ORI:
    case OP_NORI:
    case OP_XORI:
    case OP_SULTI:
    case OP_SULEI:
        printf("%s, %s, %d",
            gpr_name[einst.C.r1],
            gpr_name[einst.C.r2],
            einst.C.imm
        );
        break;
    case OP_MULI:
    case OP_IDIVI:
    case OP_IREMI:
    case OP_SEQI:
    case OP_SILTI:
    case OP_SILEI:
        printf("%s, %s, %d",
            gpr_name[einst.C.r1],
            gpr_name[einst.C.r2],
            ((i32)einst.C.imm << (32-14)) >> (32-14)
        );
        break;
    case OP_SL:
    case OP_USR:
    case OP_ISR:
    case OP_ROR:
    case OP_ROL:
        printf("%s, %s",
            gpr_name[einst.C.r1],
            gpr_name[einst.C.r2]
        );
        if (einst.C.r3 != GPR_ZR) {
            printf(", %s", gpr_name[einst.C.r3]);
        }
        if (einst.C.imm != 0) {
            printf(", %d", einst.C.imm % 64);
        }
        break;
    case OP_REV:
        printf("%s, %s, 0b%06b",
            gpr_name[einst.B.r1],
            gpr_name[einst.B.r2],
            einst.B.imm & 0b111111
        );
        break;
    case OP_SI:
    case OP_CB:
        printf("%s, %s, %d, %d",
            gpr_name[einst.B.r1],
            gpr_name[einst.B.r2],
            einst.B.imm % 64,
            (einst.B.imm >> 6) % 64
        );
        break;
    case OP_CSB:
    case OP_CLZ:
    case OP_CTZ:
        printf("%s, %s",
            gpr_name[einst.B.r1],
            gpr_name[einst.B.r2]
        );
        break;
    case OP_EXT:
    case OP_DEP:
        printf("%s, %s, %s",
            gpr_name[einst.C.r1],
            gpr_name[einst.C.r2],
            gpr_name[einst.C.r3]
        );
        break;
    case OP_BZ:
    case OP_BN:
        printf("%s, <%05zx>",
            gpr_name[einst.A.r1],
            (((i64)einst.A.imm << (64-19)) >> (64-19))*4 + location + 4
        );
        break;
    case OP_FENCE:
    case OP_SYSCALL:
    case OP_BREAKPT:
    case OP_WAIT:
    case OP_SPIN:
    case OP_IRET:
        break;
    case OP_JL:
        if (einst.B.r1 == GPR_ZR && einst.B.imm == 0) {
            if (einst.B.r2 == GPR_LP)
                break; // ret
            printf("%s", gpr_name[einst.B.r2]);
            break;
        }
        FALLTHROUGH;
    case OP_JLR:
        printf("%s, %s",
            gpr_name[einst.B.r1],
            gpr_name[einst.B.r2]
        );
        if (einst.B.imm != 0) {
            printf(", %d",
                ((i32)einst.B.imm << (32-14)) >> (32-14)
            );
        }
        break;
    case OP_LCTRL:
        printf("%s, %s",
            gpr_name[einst.A.r1],
            ctrl_name[einst.A.imm]
        );
        break;
    case OP_SCTRL:
        printf("%s, %s",
            ctrl_name[einst.A.imm],
            gpr_name[einst.A.r1]
        );
        break;
    default:
        TODO("");
    }
}

void print_encoded_inst(EncodedInst einst, usize location) {
    printf("    ");

    const char* inst_name = select_inst_name(einst);
    printf("%-5s  ", inst_name);

    print_arguments(einst, location);
}


int main(int argc, char** argv) {
    FsFile* input = fs_open(argv[1], false, false);
    string binary = fs_read_entire(input, false);

    for (usize i = 0; i < binary.len;) {
        print_line_start(i, substring_len(binary, i, min(4, binary.len - i)));

        // check if the current thing MIGHT be an instruction
        EncodedInst einst = *(EncodedInst*)&binary.raw[i];

        if (op_name[einst.opcode] != nullptr) {
            // valid!
            print_encoded_inst(einst, i);
            i += 4;
        } else {
            // invalid! print as bytes
        }

        printf("\n");
    }
}
