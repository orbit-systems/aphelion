#include "aphelion.h"

const char* const op_name[256] = {
    [OP_SSI] = "ssi",

    [OP_FENCE] = "fence",
    [OP_CINVAL] = "cinval",
    [OP_CFETCH] = "cfetch",
    [OP_JL] = "jl",
    [OP_BZ] = "bz",
    [OP_BN] = "bn",

    [OP_SYSCALL] = "syscall",
    [OP_BREAKPT] = "breakpt",
    [OP_SPIN] = "spin",
    [OP_IRET] = "iret",
    [OP_LCTRL] = "lctrl",
    [OP_SCTRL] = "sctrl",
    [OP_WAIT] = "wait",

    [OP_ADDI] = "addi",
    [OP_SUBI] = "subi",
    [OP_MULI] = "muli",
    [OP_UDIVI] = "udivi",
    [OP_IDIVI] = "idivi",
    [OP_UREMI] = "uremi",
    [OP_IREMI] = "iremi",

    [OP_ANDI] = "andi",
    [OP_ORI] = "ori",
    [OP_NORI] = "nori",
    [OP_XORI] = "xori",
    [OP_CLZ] = "clz",
    [OP_CTZ] = "ctz",
    [OP_CSB] = "csb",

    [OP_SI] = "si",
    [OP_CB] = "cb",
    [OP_REV] = "rev",

    [OP_SULTI] = "sulti",
    [OP_SILTI] = "silti",
    [OP_SULEI] = "sulei",
    [OP_SILEI] = "silei",
    [OP_SEQI] = "seqi",

    [OP_ADD] = "add",
    [OP_SUB] = "sub",
    [OP_MUL] = "mul",
    [OP_UDIV] = "udiv",
    [OP_IDIV] = "idiv",
    [OP_UREM] = "urem",
    [OP_IREM] = "irem",

    [OP_AND] = "and",
    [OP_OR] = "or",
    [OP_NOR] = "nor",
    [OP_XOR] = "xor",
    [OP_EXT] = "ext",
    [OP_DEP] = "dep",

    [OP_UMULH] = "umulh",
    [OP_IMULH] = "imulh",

    [OP_USR] = "usr",
    [OP_ISR] = "isr",
    [OP_ROR] = "ror",
    [OP_ROL] = "rol",
    [OP_SL] = "sl",

    [OP_SULT] = "sult",
    [OP_SILT] = "silt",
    [OP_SULE] = "sule",
    [OP_SILE] = "sile",
    [OP_SEQ] = "seq",

    [OP_LW] = "lw",
    [OP_LH] = "lh",
    [OP_LQ] = "lq",
    [OP_LB] = "lb",
    [OP_LLW] = "llw",
    [OP_LLH] = "llh",
    [OP_LLQ] = "llq",
    [OP_LLB] = "llb",

    [OP_SW] = "sw",
    [OP_SH] = "sh",
    [OP_SQ] = "sq",
    [OP_SB] = "sb",
    [OP_SCW] = "scw",
    [OP_SCH] = "sch",
    [OP_SCQ] = "scq",
    [OP_SCB] = "scb",
};

const char* const gpr_name[32] = {
    #define GPR(variant, name) [GPR_##variant] = name,
        APHEL_GPRS
    #undef GPR
};
