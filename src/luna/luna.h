#ifndef LUNA_H
#define LUNA_H

#include "common/type.h"

typedef enum : u8 {
    GPR_ZR = 0,
    
    GPR_A0, GPR_A1, GPR_A2, GPR_A3, GPR_A4, GPR_A5, 

    GPR_L0, GPR_L1, GPR_L2, GPR_L3, GPR_L4, GPR_L5, GPR_L6,
    GPR_L7, GPR_L8, GPR_L9, GPR_L10, GPR_L11, GPR_L12, GPR_L13,

    GPR_T0, GPR_T1, GPR_T2, GPR_T3, GPR_T4, GPR_T5,
    
    GPR_TP,
    GPR_FP,
    GPR_SP,
    GPR_LP,
    GPR_IP,
} AphelGpr;

typedef enum : u8 {
    CTRL_INT0, CTRL_INT1, CTRL_INT2, CTRL_INT3, 
    CTRL_INT4, CTRL_INT5, CTRL_INT6, CTRL_INT7,
    CTRL_INT8, CTRL_INT9, CTRL_INT10, CTRL_INT11,
    CTRL_INT12, CTRL_INT13, CTRL_INT14, CTRL_INT15,

    CTRL_INTIP,
    CTRL_INTVAL,
    CTRL_INTPTE,
    CTRL_INTCAUSE,
    CTRL_KPTP,
    CTRL_UPTP,
    CTRL_STAT,
    CTRL_INTSTAT,
} AphelCtrl;

typedef enum : u8 {
    FMT_A = 0,
    FMT_B = 1,
    FMT_C = 2,
} AphelFmt;

#define OP(major, minor, fmt) ((0b##major << 5) + (0b##minor << 2) + FMT_##fmt)
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

#define FMT_FROM_OP(opcode) (AphelFmt)((opcode) & 0b11)

typedef struct {
    AphelOpcode op;
    AphelGpr r1;
    AphelGpr r2;
    AphelGpr r3;
    i32 imm;
} AphelInst;

typedef enum : u8 {
    TOK_NEWLINE = 1,

    TOK_IDENT, // identifier
    TOK_STRING, // string literal.

    TOK_COMMA,
    TOK_COLON,
    TOK_PLUS,
    TOK_MINUS,
    TOK_MUL,
    TOK_DIV,

    TOK_OPEN_PAREN,
    TOK_CLOSE_PAREN,
    TOK_OPEN_BRACKET,
    TOK_CLOSE_BRACKET,
    TOK_OPEN_BRACE,
    TOK_CLOSE_BRACE,

    // yeah
    TOK_KW_INCLUDE,
    TOK_KW_FORCEINCLUDE,

    // flag configuration
    TOK_KW_SET,
    TOK_KW_UNSET,

    // declarations
    TOK_KW_GROUP,
    TOK_KW_SECTION,
    TOK_KW_SYMBOL,
    TOK_KW_EXTERN,
    TOK_KW_DEFINE,
    TOK_KW_ENTRY,

    // symbol binding
    TOK_KW_GLOBAL,
    TOK_KW_LOCAL,
    TOK_KW_WEAK,

    // data
    TOK_KW_REPEAT,
    TOK_KW_UNALIGNED,
    TOK_KW_ALIGN,
    TOK_KW_LOC,
    TOK_KW_ZERO,
    TOK_KW_D8,
    TOK_KW_D16,
    TOK_KW_D32,
    TOK_KW_D64,
    TOK_KW_STRING,

    // section flags
    TOK_KW_WRITABLE,
    TOK_KW_EXECUTABLE,
    TOK_KW_THREADLOCAL,
    TOK_KW_BLANK,
    TOK_KW_PIN,
    TOK_KW_COMMON,
    TOK_KW_VOLATILE,
    TOK_KW_CONCATENATE,

    // relocation flags
    TOK_KW_NOERROR,
} LunaTokenKind;

typedef struct LunaToken {
    const char* ptr;
    u16 len;
    LunaTokenKind kind;
} LunaToken;

#endif // LUNA_H
