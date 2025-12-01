#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "luna.h"
#define CSTRING_COMPATIBILITY_MODE
#include "common/type.h"
#include "common/vec.h"
#include "common/util.h"

#define TOK_CHAR(_ptr, _c) ((LunaToken) { .ptr = _ptr, .kind = _c, .len = 1})
#define TOK(_ptr, _c, _len) ((LunaToken) { .ptr = _ptr, .kind = _c, .len = _len})

extern const char* op_mnemonic[256] = {
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

// This is a sin
#define __KW_START TOK_KW_INCLUDE
extern const char* key_words[] = {
#define X(name, str) str,
	KEYWORDS_LIST
#undef X
};

/*
 * This lexer is based on a simple-ish FSM and appends tokens/data to a stream
 * with metadata in an arena (borrowed from the caller).
 *
 * Its not necessarily going to find the largest correct match (like regex),
 * but thats super okay for an assembly lexer. We can choose to quit early on
 * the first syntax error instead of trying to find them all.
 *
 * 
 * Upon successful completion, this will return 0
 * On error, this will return the offset (in bytes) from the source string for
 * error handling.
 *
 * E.g:
 *
 * size_t err_offset = 0;
 * arena_init(toks_data);
 *
 * if ((err_offset = lex(toks, toks_data, my_file)) {
 * 	// hopefully you find a better way to long than this...
 * 	eprintf("Error happened at byte %zu\n", err_offset);
 * }
 *
 */

i8 lex(Vec(LunaToken) *stream, Arena* data, const char *src) {
	const char *cur = src;
	LunaToken next = {0};

	vec_reserve(stream, 1024);

	enum : i8 {
		ERROR = -1,
		END = 0,
		DEFAULT,
		NUM,
		REAL,
		DEC,
		HEX,
		OCT,
		BIN,
		IDENT,
		STR,
		COMMENT, 
	} state = DEFAULT;

	while (*cur) {
		// Numbers generally have their data stored as the 
		// pointer since theres no reason to allocate.
		//
		// TODO: do some size comparisons to see what can fit
		// inside the f32 (e.g for 32 bit systems, we cant
		// simply just assume sizeof(void*) == sizeof(f64).

		// static keeps vars them the same across the loop, but scoped
		// to the loop unlike defining them above the while. Useful
		// for an FSM based lexer.
		static usize n = 0;

		static f64 real_f64 = 0;
		static f32 real_f32 = 0;

		switch (state) {
		case DEFAULT:
			if (*cur == ';') {
				state = COMMENT;
				continue;
			}

			switch (*cur) {
			case '\n':
				if (vec_top(stream).kind == '\n') {
					cur++;
					continue;
				} // FALLTHROUGH
			case '.':
			case ',':
			case ':':
			case '(':
			case ')':
			case '[':
			case ']':
			case '{':
			case '}':
				// this works because of explicit enum vals
				next.kind = *cur; 
				next.data = 0;
				next.len = 1;
				next.ptr = cur;

				vec_append(stream, next);
				cur++;
				continue;
			case '"':
				next.ptr = cur;
				next.kind = TOK_STRING;
				cur++;
				state = STR;
				continue;
			default:
			}

			if (isspace(*cur)) {
				cur++;
				continue;
			}

			if ('0' <= *cur && *cur <= '9') {
				next.ptr = cur;
				state = NUM;
				continue;
			}

			if (isalpha(*cur) || *cur == '_') {
				next.ptr = cur;
				next.kind = TOK_IDENT;
				state = IDENT;
				continue;
			}

			
			fprintf(stderr, "ERROR/TODO: Unrecognised char '%c', Skipping.", *cur++);
		case IDENT:
			if (*cur != '_' && !isalnum(*cur)) {
				next.len = cur - next.ptr;

				usize i;
				// The following is two very cursed linear
				// searches.
				//
				// TODO: replace with a perfect
				// hashmap for keywords that map to integers
				// with a bit set to determine the kind as
				// TOK_OP and TOK_KW_
				for (i = 0; i < len(op_mnemonic); i++) {
					if (op_mnemonic[i] != NULL
					    && next.len == strlen(op_mnemonic[i])
					    && strncmp(next.ptr, op_mnemonic[i], next.len) == 0) {
						next.data = (void *) i;
						next.kind = TOK_OP;
						vec_append(stream, next);
						break;
					}
				}

				if (i != len(op_mnemonic)) {
					state = DEFAULT;
					continue;
				}

				for (i = 0; i < len(key_words); i++) {
					if (key_words[i] != NULL
					    && next.len == strlen(key_words[i])
					    && strncmp(next.ptr, key_words[i], next.len) == 0) {
						next.data = NULL;
						next.kind = (LunaTokenKind) (i + __KW_START);
						vec_append(stream, next);
						break;
					}
				}

				if (i != len(key_words)) {
					state = DEFAULT;
					continue;
				}

				char *s = arena_alloc(data, next.len + 1, 1);
				memcpy(s, next.ptr, next.len);
				s[next.len] = '\0';
				next.data = s;
				
				vec_append(stream, next);
				state = DEFAULT;
				continue;
			}

			cur++;

			continue;
		case COMMENT:
			cur++;
			if (*cur == '\n') {
				state = DEFAULT;
			}
			continue;
		case STR:
			// Maybe an vec_init_arena would be nice or a string 
			// where it allocates using arena_alloc.

			// Doesnt handle escaping quotes yet.
			if (*cur == '"') {
				cur++;
				next.len = cur - next.ptr;

				if (next.len != 2) {
					char *s = arena_alloc(data, next.len - 1, 1);
					// Copy without starting and ending quotes
					memcpy(s, next.ptr + 1, next.len - 2);
					s[next.len - 1] = '\0';
					next.data = s;
					// TODO: add some way to register 
					// string_free callback in arena.
					// Right now this leaks.
					vec_append(stream, next);
				}
				
				state = DEFAULT;
				continue;
			}

			cur++;
			continue;
		case NUM:
			next.kind = TOK_NUM;
			n = 0;

			real_f64 = 0;
			real_f32 = 0;

			if (*cur == '0') {
				cur++;
				switch (*cur) {
				case '.':
					cur++;
					state = REAL;
					continue;
				case 'x':
					cur++;
					state = HEX;
					continue;
				case 'o':
					cur++;
					state = OCT;
					continue;
				case 'b':
					cur++;
					state = BIN;
					continue;
				}
			} else if ('0' < *cur && *cur <= '9') {
				state = DEC;
				continue;
			}
		case BIN:
			if (*cur == '_') {
				cur++;
				continue;
			}

			if (*cur != '0' && *cur != '1') {
				state = DEFAULT;
				next.data = (void *)n;
				next.len = cur - next.ptr;
				vec_append(stream, next);
				continue;
			}


			n <<= 1;
			n |= (*cur - '0');
			cur++;
			continue;
		case HEX:
			if (*cur == '_') {
				cur++;
				continue;
			}
			
			if (!isxdigit(*cur)) {
				state = DEFAULT;
				next.data = (void *)n;
				next.len = cur - next.ptr;
				vec_append(stream, next);
				continue;
			}

			u8 val;
			if ('0' <= *cur && *cur <= '9') {
				val = *cur - '0';
			}
			else if ('a' <= *cur && *cur <= 'f') {
				val = *cur - 'a' + 10;
			}
			else if ('A' <= *cur && *cur <= 'F') {
				val = *cur - 'A' + 10;
			}

			n <<= 4; // recall one hex digit is a nibble
			n |= val;

			cur++;
			continue;
		case OCT:
			if (*cur == '_') {
				cur++;
				continue;
			}
			
			if ('0' > *cur || *cur > '7') {
				state = DEFAULT;
				next.data = (void *)n;
				next.len = cur - next.ptr;
				vec_append(stream, next);
				continue;
			}

			n <<= 3;
			n |= (*cur - '0');

			cur++;
			continue;
		case DEC:
			if (*cur == '_') {
				cur++;
				continue;
			}
			
			// leading zeros/digits mislead us once again...
			if (*cur == '.') {
				cur++;

				real_f32 = (f32) n;
				real_f64 = (f32) n;

				state = REAL;
				continue;
			}

			if ('0' > *cur || *cur > '9') {
				state = DEFAULT;
				next.data = (void *)n;
				next.len = cur - next.ptr;
				vec_append(stream, next);
				continue;
			}

			n *= 10;
			n += (*cur - '0');

			cur++;
			continue;
		case REAL:
			/* TODO: read real into f32/f64
			Its kind of difficult to read reals accurately from
			text without introducing innacuracies from fp ops.
			This needs looking into and actual technical 
			knowledge.
			
			Eisel-Lemire algorithm / Ryu-Parse:
			    faster, more edge cases, require arbitrary
			    precision. Used in go/rust stdlibs. Alot of
			    complexity
			
			David Gay’s algorithm:
			    slower, but used for 30 years in glibc,
			    modified version in musl. Its your typical
			    strtod on almost anything but embedded.
			    Kind of free complexity wise if we use libc
			
			TL;DR
			Ill use strtod for now. We can always change. 
			Just note its a speed trade off
			*/

			/* HALF-IMPL
			string s = strprintf("%d.%de%d", n);
			char *end; = s.raw + s.len;

			f64 = strtod(s.raw, end);
			if (end != s.raw + s.len)
				// error
			
			// somehow check precision:
			// 0.0f 0.0d
			next.data = (void *)real_f64;
			// OR
			next.data = (void *)real_f32;
			string_free(s);
			*/

			n = 0;
			state = DEFAULT;
			continue;
		}
	}
	return END;
}
