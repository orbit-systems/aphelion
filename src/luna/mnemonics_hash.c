/* ANSI-C code produced by gperf version 3.3 */
/* Command-line: gperf -C -t -K name -D ops.gperf  */
/* Computed positions: -k'1-2,4,$' */

#if !((' ' == 32) && ('!' == 33) && ('"' == 34) && ('#' == 35) \
      && ('%' == 37) && ('&' == 38) && ('\'' == 39) && ('(' == 40) \
      && (')' == 41) && ('*' == 42) && ('+' == 43) && (',' == 44) \
      && ('-' == 45) && ('.' == 46) && ('/' == 47) && ('0' == 48) \
      && ('1' == 49) && ('2' == 50) && ('3' == 51) && ('4' == 52) \
      && ('5' == 53) && ('6' == 54) && ('7' == 55) && ('8' == 56) \
      && ('9' == 57) && (':' == 58) && (';' == 59) && ('<' == 60) \
      && ('=' == 61) && ('>' == 62) && ('?' == 63) && ('A' == 65) \
      && ('B' == 66) && ('C' == 67) && ('D' == 68) && ('E' == 69) \
      && ('F' == 70) && ('G' == 71) && ('H' == 72) && ('I' == 73) \
      && ('J' == 74) && ('K' == 75) && ('L' == 76) && ('M' == 77) \
      && ('N' == 78) && ('O' == 79) && ('P' == 80) && ('Q' == 81) \
      && ('R' == 82) && ('S' == 83) && ('T' == 84) && ('U' == 85) \
      && ('V' == 86) && ('W' == 87) && ('X' == 88) && ('Y' == 89) \
      && ('Z' == 90) && ('[' == 91) && ('\\' == 92) && (']' == 93) \
      && ('^' == 94) && ('_' == 95) && ('a' == 97) && ('b' == 98) \
      && ('c' == 99) && ('d' == 100) && ('e' == 101) && ('f' == 102) \
      && ('g' == 103) && ('h' == 104) && ('i' == 105) && ('j' == 106) \
      && ('k' == 107) && ('l' == 108) && ('m' == 109) && ('n' == 110) \
      && ('o' == 111) && ('p' == 112) && ('q' == 113) && ('r' == 114) \
      && ('s' == 115) && ('t' == 116) && ('u' == 117) && ('v' == 118) \
      && ('w' == 119) && ('x' == 120) && ('y' == 121) && ('z' == 122) \
      && ('{' == 123) && ('|' == 124) && ('}' == 125) && ('~' == 126))
/* The character set is not based on ISO-646.  */
#error "gperf generated tables don't work with this execution character set. Please report a bug to <bug-gperf@gnu.org>."
#endif

#line 1 "ops.gperf"

#include "luna.h"
#line 4 "ops.gperf"
struct op_entry { const char *name; AphelOpcode opcode; };

#define TOTAL_KEYWORDS 77
#define MIN_WORD_LENGTH 2
#define MAX_WORD_LENGTH 7
#define MIN_HASH_VALUE 13
#define MAX_HASH_VALUE 203
/* maximum key range = 191, duplicates = 0 */

#ifdef __GNUC__
__inline
#else
#ifdef __cplusplus
inline
#endif
#endif
static unsigned int
hash (register const char *str, register size_t len)
{
  static const unsigned char asso_values[] =
    {
      204, 204, 204, 204, 204, 204, 204, 204, 204, 204,
      204, 204, 204, 204, 204, 204, 204, 204, 204, 204,
      204, 204, 204, 204, 204, 204, 204, 204, 204, 204,
      204, 204, 204, 204, 204, 204, 204, 204, 204, 204,
      204, 204, 204, 204, 204, 204, 204, 204, 204, 204,
      204, 204, 204, 204, 204, 204, 204, 204, 204, 204,
      204, 204, 204, 204, 204, 204, 204, 204, 204, 204,
      204, 204, 204, 204, 204, 204, 204, 204, 204, 204,
      204, 204, 204, 204, 204, 204, 204, 204, 204, 204,
      204, 204, 204, 204, 204, 204, 204,  95,  10,   5,
       15,  10,  40,   0,  80,   5,  15, 204,   0,  25,
       25,  40,  90,  45,  50,  40,   0,  10,   0,  55,
       60,   0,  22,   0, 204, 204, 204, 204, 204, 204,
      204, 204, 204, 204, 204, 204, 204, 204, 204, 204,
      204, 204, 204, 204, 204, 204, 204, 204, 204, 204,
      204, 204, 204, 204, 204, 204, 204, 204, 204, 204,
      204, 204, 204, 204, 204, 204, 204, 204, 204, 204,
      204, 204, 204, 204, 204, 204, 204, 204, 204, 204,
      204, 204, 204, 204, 204, 204, 204, 204, 204, 204,
      204, 204, 204, 204, 204, 204, 204, 204, 204, 204,
      204, 204, 204, 204, 204, 204, 204, 204, 204, 204,
      204, 204, 204, 204, 204, 204, 204, 204, 204, 204,
      204, 204, 204, 204, 204, 204, 204, 204, 204, 204,
      204, 204, 204, 204, 204, 204, 204, 204, 204, 204,
      204, 204, 204, 204, 204, 204, 204, 204, 204, 204,
      204, 204, 204, 204, 204, 204, 204
    };
  register unsigned int hval = len;

  switch (hval)
    {
      default:
        hval += asso_values[(unsigned char)str[3]];
#if (defined __cplusplus && (__cplusplus >= 201703L || (__cplusplus >= 201103L && defined __clang__ && __clang_major__ + (__clang_minor__ >= 9) > 3))) || (defined __STDC_VERSION__ && __STDC_VERSION__ >= 202000L && ((defined __GNUC__ && __GNUC__ >= 10) || (defined __clang__ && __clang_major__ >= 9)))
      [[fallthrough]];
#elif (defined __GNUC__ && __GNUC__ >= 7) || (defined __clang__ && __clang_major__ >= 10)
      __attribute__ ((__fallthrough__));
#endif
      /*FALLTHROUGH*/
      case 3:
      case 2:
        hval += asso_values[(unsigned char)str[1]+1];
#if (defined __cplusplus && (__cplusplus >= 201703L || (__cplusplus >= 201103L && defined __clang__ && __clang_major__ + (__clang_minor__ >= 9) > 3))) || (defined __STDC_VERSION__ && __STDC_VERSION__ >= 202000L && ((defined __GNUC__ && __GNUC__ >= 10) || (defined __clang__ && __clang_major__ >= 9)))
      [[fallthrough]];
#elif (defined __GNUC__ && __GNUC__ >= 7) || (defined __clang__ && __clang_major__ >= 10)
      __attribute__ ((__fallthrough__));
#endif
      /*FALLTHROUGH*/
      case 1:
        hval += asso_values[(unsigned char)str[0]];
        break;
    }
  return hval + asso_values[(unsigned char)str[len - 1]];
}

const struct op_entry *
in_word_set (register const char *str, register size_t len)
{
#if (defined __GNUC__ && __GNUC__ + (__GNUC_MINOR__ >= 6) > 4) || (defined __clang__ && __clang_major__ >= 3)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
#endif
  static const struct op_entry wordlist[] =
    {
#line 53 "ops.gperf"
      {"ext", OP_EXT},
#line 70 "ops.gperf"
      {"lb", OP_LB},
#line 33 "ops.gperf"
      {"csb", OP_CSB},
#line 46 "ops.gperf"
      {"idiv", OP_IDIV},
#line 35 "ops.gperf"
      {"cb", OP_CB},
#line 45 "ops.gperf"
      {"udiv", OP_UDIV},
#line 24 "ops.gperf"
      {"idivi", OP_IDIVI},
#line 8 "ops.gperf"
      {"cinval", OP_CINVAL},
#line 44 "ops.gperf"
      {"mul", OP_MUL},
#line 23 "ops.gperf"
      {"udivi", OP_UDIVI},
#line 11 "ops.gperf"
      {"bz", OP_BZ},
#line 74 "ops.gperf"
      {"llb", OP_LLB},
#line 22 "ops.gperf"
      {"muli", OP_MULI},
#line 32 "ops.gperf"
      {"ctz", OP_CTZ},
#line 10 "ops.gperf"
      {"jl", OP_JL},
#line 62 "ops.gperf"
      {"sult", OP_SULT},
#line 6 "ops.gperf"
      {"ssi", OP_SSI},
#line 16 "ops.gperf"
      {"iret", OP_IRET},
#line 37 "ops.gperf"
      {"sulti", OP_SULTI},
#line 43 "ops.gperf"
      {"sub", OP_SUB},
#line 21 "ops.gperf"
      {"subi", OP_SUBI},
#line 31 "ops.gperf"
      {"clz", OP_CLZ},
#line 78 "ops.gperf"
      {"sb", OP_SB},
#line 58 "ops.gperf"
      {"isr", OP_ISR},
#line 63 "ops.gperf"
      {"silt", OP_SILT},
#line 39 "ops.gperf"
      {"sulei", OP_SULEI},
#line 34 "ops.gperf"
      {"si", OP_SI},
#line 57 "ops.gperf"
      {"usr", OP_USR},
#line 64 "ops.gperf"
      {"sule", OP_SULE},
#line 38 "ops.gperf"
      {"silti", OP_SILTI},
#line 61 "ops.gperf"
      {"sl", OP_SL},
#line 82 "ops.gperf"
      {"scb", OP_SCB},
#line 19 "ops.gperf"
      {"wait", OP_WAIT},
#line 17 "ops.gperf"
      {"lctrl", OP_LCTRL},
#line 73 "ops.gperf"
      {"llq", OP_LLQ},
#line 13 "ops.gperf"
      {"syscall", OP_SYSCALL},
#line 40 "ops.gperf"
      {"silei", OP_SILEI},
#line 12 "ops.gperf"
      {"bn", OP_BN},
#line 65 "ops.gperf"
      {"sile", OP_SILE},
#line 26 "ops.gperf"
      {"iremi", OP_IREMI},
#line 71 "ops.gperf"
      {"llw", OP_LLW},
#line 25 "ops.gperf"
      {"uremi", OP_UREMI},
#line 68 "ops.gperf"
      {"lh", OP_LH},
#line 28 "ops.gperf"
      {"ori", OP_ORI},
#line 9 "ops.gperf"
      {"cfetch", OP_CFETCH},
#line 36 "ops.gperf"
      {"rev", OP_REV},
#line 41 "ops.gperf"
      {"seqi", OP_SEQI},
#line 69 "ops.gperf"
      {"lq", OP_LQ},
#line 48 "ops.gperf"
      {"irem", OP_IREM},
#line 7 "ops.gperf"
      {"fence", OP_FENCE},
#line 81 "ops.gperf"
      {"scq", OP_SCQ},
#line 47 "ops.gperf"
      {"urem", OP_UREM},
#line 72 "ops.gperf"
      {"llh", OP_LLH},
#line 18 "ops.gperf"
      {"sctrl", OP_SCTRL},
#line 79 "ops.gperf"
      {"scw", OP_SCW},
#line 56 "ops.gperf"
      {"imulh", OP_IMULH},
#line 67 "ops.gperf"
      {"lw", OP_LW},
#line 20 "ops.gperf"
      {"addi", OP_ADDI},
#line 55 "ops.gperf"
      {"umulh", OP_UMULH},
#line 42 "ops.gperf"
      {"add", OP_ADD},
#line 76 "ops.gperf"
      {"sh", OP_SH},
#line 66 "ops.gperf"
      {"seq", OP_SEQ},
#line 29 "ops.gperf"
      {"nori", OP_NORI},
#line 50 "ops.gperf"
      {"or", OP_OR},
#line 77 "ops.gperf"
      {"sq", OP_SQ},
#line 80 "ops.gperf"
      {"sch", OP_SCH},
#line 15 "ops.gperf"
      {"spin", OP_SPIN},
#line 60 "ops.gperf"
      {"rol", OP_ROL},
#line 54 "ops.gperf"
      {"dep", OP_DEP},
#line 27 "ops.gperf"
      {"andi", OP_ANDI},
#line 14 "ops.gperf"
      {"breakpt", OP_BREAKPT},
#line 49 "ops.gperf"
      {"and", OP_AND},
#line 75 "ops.gperf"
      {"sw", OP_SW},
#line 30 "ops.gperf"
      {"xori", OP_XORI},
#line 51 "ops.gperf"
      {"nor", OP_NOR},
#line 59 "ops.gperf"
      {"ror", OP_ROR},
#line 52 "ops.gperf"
      {"xor", OP_XOR}
    };
#if (defined __GNUC__ && __GNUC__ + (__GNUC_MINOR__ >= 6) > 4) || (defined __clang__ && __clang_major__ >= 3)
#pragma GCC diagnostic pop
#endif

  static const signed char lookup[] =
    {
      -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,  0,
      -1, -1, -1,  1,  2,  3, -1, -1,  4, -1,  5,  6,  7, -1,
       8, -1,  9, -1, -1, -1, 10, -1, -1, -1, 11, 12, 13, -1,
      14, -1, 15, -1, -1, -1, 16, 17, 18, -1, -1, 19, 20, 21,
      -1, 22, 23, 24, 25, -1, 26, 27, 28, 29, -1, 30, 31, 32,
      33, -1, -1, 34, 35, 36, -1, 37, -1, 38, 39, -1, -1, 40,
      -1, 41, -1, 42, 43, -1, -1, 44, -1, 45, 46, -1, -1, 47,
      -1, 48, 49, -1, -1, 50, 51, -1, -1, -1, 52, -1, 53, -1,
      -1, 54, -1, 55, -1, 56, -1, 57, 58, -1, -1, 59, -1, -1,
      -1, 60, 61, 62, -1, -1, 63, -1, -1, -1, -1, 64, 65, 66,
      -1, -1, -1, 67, -1, -1, -1, -1, 68, 69, -1, -1, 70, 71,
      -1, -1, -1, 72, -1, -1, -1, -1, -1, -1, 73, -1, -1, -1,
      74, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 75, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, 76
    };

  if (len <= MAX_WORD_LENGTH && len >= MIN_WORD_LENGTH)
    {
      register unsigned int key = hash (str, len);

      if (key <= MAX_HASH_VALUE)
        {
          register int index = lookup[key];

          if (index >= 0)
            {
              register const char *s = wordlist[index].name;

              if (*str == *s && !strcmp (str + 1, s + 1))
                return &wordlist[index];
            }
        }
    }
  return (struct op_entry *) 0;
}
#line 83 "ops.gperf"

