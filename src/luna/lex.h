#ifndef LUNA_LEX_H
#define LUNA_LEX_H

#include "common/type.h"
#include "luna.h"

#define LUNA_KEYWORDS \
    /* content inclusion */ \
    KW(INCLUDE, "include") \
    KW(FORCEINCLUDE, "forceinclude") \
    /* declarations */\
    KW(SECTION, "section") \
    KW(SYMBOL,  "symbol") \
    KW(DEFINE,  "define") \
    KW(LOC,     "loc") \
    /* symbol modifiers */ \
    KW(ENTRY, "entry") \
    /* symbol binding */ \
    KW(GLOBAL, "global") \
    KW(LOCAL,  "local") \
    KW(WEAK,   "weak") \
    /* data modifiers */ \
    KW(REPEAT,    "repeat") \
    KW(UNALIGNED, "unaligned") \
    /* data */ \
    KW(ALIGN,  "align") \
    KW(ZERO,   "zero") \
    KW(BYTE,   "byte") \
    KW(QWORD,  "qword") \
    KW(HWORD,  "hword") \
    KW(WORD,   "word") \
    KW(STRING, "string") \
    /* section flags */ \
    KW(UNMAPPED,    "unmapped") \
    KW(WRITABLE,    "writable") \
    KW(EXECUTABLE,  "executable") \
    KW(THREADLOCAL, "threadlocal") \
    KW(BLANK,       "blank") \
    KW(PINNED,      "pinned") \
    KW(COMMON,      "common") \
    KW(NONVOLATILE, "nonvolatile") \
    KW(UNIQUE,      "unique") \

typedef enum : u8 {
    TOK_INVALID = 0,

    TOK_EOF, /// End of file.

    TOK_IDENT, /// Identifier.

    /// String literal. If contains escape sequences,
    /// its subtype is TOK_STR_HAS_ESCAPE.
    TOK_STR_LIT,

    /// Character literal. If contains escape sequences,
    /// its subtype is TOK_STR_HAS_ESCAPE.
    TOK_CHAR_LIT,

    /// Numeric literal.
    TOK_NUM_LIT,

    TOK_INST, /// Instruction name.
    TOK_GPR,  /// General-purpose register name.
    TOK_CTRL, /// Control register name.

    TOK_NEWLINE = '\n',

    TOK_OPEN_PAREN    = '(',
    TOK_CLOSE_PAREN   = ')',
    TOK_OPEN_BRACKET  = '[',
    TOK_CLOSE_BRACKET = ']',
    TOK_OPEN_BRACE    = '{',
    TOK_CLOSE_BRACE   = '}',

    TOK_COMMA = ',',
    TOK_COLON = ':',
    TOK_EQ    = '=',

    TOK_PLUS  = '+',
    TOK_MINUS = '-',
    TOK_MUL   = '*',
    TOK_DIV   = '/',
    TOK_REM   = '%',
    TOK_AND   = '&',
    TOK_OR    = '|',
    TOK_TILDE = '~',

    #define KW(variant, name) TOK_KW_##variant,
        LUNA_KEYWORDS
    #undef KW

} TokenKind;
#define TOK_STR_HAS_ESCAPE 1

extern const char* const token_kind_name[];

#ifdef HOST_POINTER_48_BITS
    /// A token of text from a file.
    /// Note that length is not stored, as it can be cheaply recalculated if needed.
    /// It is often not needed, as the token kind is enough for most cases.
    typedef struct Token {
        /// The token's kind. (TokenKind)
        u64 kind : 8;
        /// Subkind, if applicable. Inst name, GPR, control register, etc.
        u64 subkind : 8;
        /// First 48 bits of a pointer to the first character in the text.
        i64 raw : 48;
    } Token;
#else
    /// A token of text from a file.
    /// Note that length is not stored, as it can be cheaply recalculated if needed.
    /// It is often not needed, as the token kind is enough for most cases.
    typedef struct Token {
        /// The token's kind.
        TokenKind kind;
        /// Subkind, if applicable. Inst name, GPR, control register, etc.
        u8 subtype;
        /// First 48 bits of a pointer to the first character in the text.
        /// \note Owned by the `SourceFile`, which is owned by the `MarsCompiler` object.
        char* raw;
    } Token;
#endif


#define INST_NAMES \
    INST(LW,  "lw") \
    INST(LH,  "lh") \
    INST(LQ,  "lq") \
    INST(LB,  "lb") \
    INST(LLW, "llw") \
    INST(LLH, "llh") \
    INST(LLQ, "llq") \
    INST(LLB, "llb") \
    \
    INST(SW,  "sw") \
    INST(SH,  "sh") \
    INST(SQ,  "sq") \
    INST(SB,  "sb") \
    INST(SCW, "scw") \
    INST(SCH, "sch") \
    INST(SCQ, "scq") \
    INST(SCB, "scb") \
    \
    INST(FENCE,    "fence") \
    INST(FENCE_S,  "fence.s") \
    INST(FENCE_L,  "fence.l") \
    \
    INST(CINVAL_BLOCK,   "cinval.block") \
    INST(CINVAL_PAGE,    "cinval.page") \
    INST(CINVAL_ALL,     "cinval.all") \
    INST(CINVAL_I_BLOCK, "cinval.i.block") \
    INST(CINVAL_I_PAGE,  "cinval.i.page") \
    INST(CINVAL_I_ALL,   "cinval.i.all") \
    INST(CINVAL_D_BLOCK, "cinval.d.block") \
    INST(CINVAL_D_PAGE,  "cinval.d.page") \
    INST(CINVAL_D_ALL,   "cinval.d.all") \
    \
    INST(CFETCH_L,   "cfetch.l") \
    INST(CFETCH_S,   "cfetch.s") \
    INST(CFETCH_I,   "cfetch.i") \
    INST(CFETCH_LS,  "cfetch.ls") \
    INST(CFETCH_LI,  "cfetch.li") \
    INST(CFETCH_SI,  "cfetch.si") \
    INST(CFETCH_LSI, "cfetch.lsi") \
    \
    INST(SSI,   "ssi") \
    INST(SSI_C, "ssi.c") \
    \
    INST(ADD,   "add") \
    INST(SUB,   "sub") \
    INST(MUL,   "mul") \
    INST(UMULH, "umulh") \
    INST(IMULH, "imulh") \
    INST(UDIV,  "udiv") \
    INST(IDIV,  "idiv") \
    INST(UREM,  "urem") \
    INST(IREM,  "irem") \
    \
    INST(AND, "and") \
    INST(OR,  "or") \
    INST(NOR, "nor") \
    INST(XOR, "xor") \
    \
    INST(ADDI,  "addi") \
    INST(SUBI,  "subi") \
    INST(MULI,  "muli") \
    INST(UDIVI, "udivi") \
    INST(IDIVI, "idivi") \
    INST(UREMI, "uremi") \
    INST(IREMI, "iremi") \
    \
    INST(ANDI, "andi") \
    INST(ORI,  "ori") \
    INST(NORI, "nori") \
    INST(XORI, "xori") \
    \
    INST(SL,   "sl") \
    INST(USR,  "usr") \
    INST(ISR,  "isr") \
    INST(SI_U, "si.u") \
    INST(SI_I, "si.i") \
    INST(CB,   "cb") \
    INST(ROR,  "ror") \
    INST(ROL,  "rol") \
    \
    INST(REV,     "rev") \
    INST(REV_H,   "rev.h") \
    INST(REV_Q,   "rev.q") \
    INST(REV_B,   "rev.b") \
    INST(REV_BIT, "rev.bit") \
    \
    INST(CSB, "csb") \
    INST(CLZ, "clz") \
    INST(CTZ, "ctz") \
    INST(EXT, "ext") \
    INST(DEP, "dep") \
    \
    INST(SEQ,  "seq") \
    INST(SULT, "sult") \
    INST(SILT, "silt") \
    INST(SULE, "sule") \
    INST(SILE, "sile") \
    \
    INST(SEQI,  "seqi") \
    INST(SULTI, "sulti") \
    INST(SILTI, "silti") \
    INST(SULEI, "sulei") \
    INST(SILEI, "silei") \
    \
    INST(BZ, "bz") \
    INST(BN, "bn") \
    \
    INST(JL,  "jl") \
    INST(JLR, "jlr") \
    \
    INST(SYSCALL,  "syscall") \
    INST(BREAKPT,  "breakpt") \
    INST(WAIT,  "wait") \
    INST(SPIN,  "spin") \
    INST(IRET,  "iret") \
    INST(LCTRL,  "lctrl") \
    INST(SCTRL,  "sctrl") \
    \
    /* Psuedo-instructions are prefixed with P_ */ \
    INST(P_CALL,    "call") \
    INST(P_SCALL,   "scall") \
    INST(P_RCALL,   "rcall") \
    INST(P_FCALL,   "fcall") \
    INST(P_RET,     "ret") \
    INST(P_NOP,     "nop") \
    INST(P_MOV,     "mov") \
    INST(P_LI,      "li") \

/// Every kind of instruction name accepted by the assembler.
/// This includes instructions with configuration annotations,
/// Pseudo-instructions, etc.
/// \note THIS IS DIFFERENT FROM `AphelOpcode`.
typedef enum : u8 {
    INST__INVALID = 0,

    #define INST(variant, name) INST_##variant,
        INST_NAMES
    #undef INST

    INST__COUNT
} InstName;

extern const char* const inst_name_string[INST__COUNT];

/// Return the pointer to the character a token `t` starts at.
static inline const char* token_start(Token t) {
    return (const char*)(isize)t.raw;
}

/// A lexer for making tokens from text.
typedef struct Lexer {
    /// Current byte the lexer is processing.
    usize cursor;

    /// Reference to `current_file`s source text.
    /// \note Owned by the `SourceFile`, which is owned by the `LunaInstance` object.
    string source;

    /// ID of current file the lexer is operating on.
    SourceFileId current_file;

    /// List of tokens generated from the source text.
    Vec(Token) tokens;

    /// The assembler instance this lexer was spawned from.
    LunaInstance* luna;
} Lexer;

/// Create a lexer on source file `file` attached to luna instance `luna`.
Lexer lexer_new(LunaInstance* luna, SourceFileId file);

/// Destroy lexer `l` and return its token list.
Vec(Token) lexer_destroy_to_tokens(Lexer* l);

/// Scan lexer `l`'s source text for the next token.
/// \return Whether a new token can be generated from a subsequent call.
bool lexer_next_token(Lexer* l);

#endif // LUNA_LEX_H
