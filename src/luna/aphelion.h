#ifndef APHELION_H
#define APHELION_H

#include "common/type.h"

#define APHEL_GPRS \
    GPR(ZR, "zr") \
    \
    GPR(A0, "a0") GPR(A1, "a1") GPR(A2, "a2") \
    GPR(A3, "a3") GPR(A4, "a4") GPR(A5, "a5") \
    \
    GPR(L0, "l0") GPR(L1, "l1") GPR(L2, "l2") \
    GPR(L3, "l3") GPR(L4, "l4") GPR(L5, "l5") \
    GPR(L6, "l6") GPR(L7, "l7") GPR(L8, "l8") \
    GPR(L9, "l9") GPR(L10, "l10") GPR(L11, "l11") \
    GPR(L12, "l12") GPR(L13, "l13") \
    \
    GPR(T0, "t0") GPR(T1, "t1") GPR(T2, "t2") \
    GPR(T3, "t3") GPR(T4, "t4") GPR(T5, "t5") \
    \
    GPR(TP, "tp") GPR(FP, "fp") GPR(SP, "sp") \
    GPR(LP, "lp") GPR(IP, "ip") \


/// Aphelion general-purpose register (GPR).
typedef enum AphelGpr : u8 {
    #define GPR(variant, name) GPR_##variant,
        APHEL_GPRS
    #undef GPR
} AphelGpr;

#define APHEL_CTRLS \
    CTRL(INT0, "int0") CTRL(INT1, "int1") CTRL(INT2, "int2") \
    CTRL(INT3, "int3") CTRL(INT4, "int4") CTRL(INT5, "int5") \
    CTRL(INT6, "int6") CTRL(INT7, "int7") CTRL(INT8, "int8") \
    CTRL(INT9, "int9") CTRL(INT10, "int10") CTRL(INT12, "int12") \
    CTRL(INT13, "int13") CTRL(INT14, "int14") CTRL(INT15, "int15") \
    \
    CTRL(INTIP, "intip") \
    CTRL(INTVAL, "intval") \
    CTRL(INTPTE, "intpte") \
    CTRL(INTCAUSE, "intcause") \
    CTRL(KPTP, "kptp") \
    CTRL(UPTP, "uptp") \
    CTRL(STAT, "stat") \
    CTRL(INTSTAT, "intstat") \

/// Aphelion control register.
typedef enum AphelCtrl : u8 {
     #define CTRL(variant, name) CTRL_##variant,
        APHEL_CTRLS
    #undef CTRL
} AphelCtrl;

/// Aphelion instruction format.
typedef enum : u8 {
    FMT_A = 0,
    FMT_B = 1,
    FMT_C = 2,
} AphelFmt;

#define OP(major, minor, fmt) ((0b##major << 5) + (0b##minor << 2) + FMT_##fmt)
// Instruction opcode.
typedef enum : u8 {
    OP_SSI = OP(010, 000, A),

    OP_FENCE  = OP(100, 000, A),
    OP_CINVAL = OP(100, 001, A),
    OP_CFETCH = OP(100, 010, A),
    OP_JL     = OP(100, 101, A),
    OP_BZ     = OP(100, 110, A),
    OP_BN     = OP(100, 111, A),

    OP_SYSCALL = OP(111, 000, A),
    OP_BREAKPT = OP(111, 001, A),
    OP_SPIN    = OP(111, 010, A),
    OP_IRET    = OP(111, 100, A),
    OP_LCTRL   = OP(111, 101, A),
    OP_SCTRL   = OP(111, 110, A),
    OP_WAIT    = OP(111, 111, A),

    OP_ADDI  = OP(000, 000, B),
    OP_SUBI  = OP(000, 001, B),
    OP_MULI  = OP(000, 010, B),
    OP_UDIVI = OP(000, 100, B),
    OP_IDIVI = OP(000, 101, B),
    OP_UREMI = OP(000, 110, B),
    OP_IREMI = OP(000, 111, B),

    OP_ANDI = OP(001, 000, B),
    OP_ORI  = OP(001, 001, B),
    OP_NORI = OP(001, 010, B),
    OP_XORI = OP(001, 011, B),
    OP_CLZ  = OP(001, 100, B),
    OP_CTZ  = OP(001, 101, B),
    OP_CSB  = OP(001, 110, B),

    OP_SI  = OP(010, 000, B),
    OP_CB  = OP(010, 001, B),
    OP_REV = OP(010, 010, B),

    OP_SULTI = OP(011, 001, B),
    OP_SILTI = OP(011, 010, B),
    OP_SULEI = OP(011, 011, B),
    OP_SILEI = OP(011, 100, B),
    OP_SEQI  = OP(011, 101, B),

    OP_ADD  = OP(000, 000, C),
    OP_SUB  = OP(000, 001, C),
    OP_MUL  = OP(000, 010, C),
    OP_UDIV = OP(000, 100, C),
    OP_IDIV = OP(000, 101, C),
    OP_UREM = OP(000, 110, C),
    OP_IREM = OP(000, 111, C),

    OP_AND   = OP(001, 000, C),
    OP_OR    = OP(001, 001, C),
    OP_NOR   = OP(001, 010, C),
    OP_XOR   = OP(001, 011, C),
    OP_EXT   = OP(001, 100, C),
    OP_DEP   = OP(001, 101, C),
    OP_UMULH = OP(001, 110, C),
    OP_IMULH = OP(001, 111, C),

    OP_USR  = OP(010, 000, C),
    OP_ISR  = OP(010, 001, C),
    OP_ROR  = OP(010, 010, C),
    OP_ROL  = OP(010, 011, C),
    OP_SL   = OP(010, 100, C),

    OP_SULT = OP(011, 001, C),
    OP_SILT = OP(011, 010, C),
    OP_SULE = OP(011, 011, C),
    OP_SILE = OP(011, 100, C),
    OP_SEQ  = OP(011, 101, C),

    OP_LW  = OP(100, 000, C),
    OP_LH  = OP(100, 001, C),
    OP_LQ  = OP(100, 010, C),
    OP_LB  = OP(100, 011, C),
    OP_LLW = OP(100, 100, C),
    OP_LLH = OP(100, 101, C),
    OP_LLQ = OP(100, 110, C),
    OP_LLB = OP(100, 111, C),

    OP_SW  = OP(101, 000, C),
    OP_SH  = OP(101, 001, C),
    OP_SQ  = OP(101, 010, C),
    OP_SB  = OP(101, 011, C),
    OP_SCW = OP(101, 100, C),
    OP_SCH = OP(101, 101, C),
    OP_SCQ = OP(101, 110, C),
    OP_SCB = OP(101, 111, C),
} AphelOpcode;
#undef OP

/// Get the instruction format associated with AphelOpcode `opcode`.
static inline AphelFmt fmt_from_op(AphelOpcode opcode) {
    return (AphelFmt)((opcode) & 0b11);
}

/// An 'assembled' but unencoded Aphelion instruction.
typedef struct {
    AphelOpcode op;
    AphelGpr r1;
    AphelGpr r2;
    AphelGpr r3;
    i32 imm;
} AphelInst;

/// Name/name for an opcode. If an opcode is not present, gives `nullptr`.
extern const char* const op_name[256];

#endif // APHELION_H
