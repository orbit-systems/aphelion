#include "common/type.h"
#include "common/vec.h"
#include "luna.h"
#include "lex.h"

#include <stdio.h>

int main() {
	Vec(LunaToken) out;
	vec_init(&out, 128);

	const char *src =
		"executable \n"
		"section \"text\"\n"
		"global start:\n"
		"    mov l0, lr\n"
		"    call lr, increment\n"
		"    mov lr, l0\n"
		"    li t0, mydata\n"
		"    lw a0, [t0]\n"
		"    ret\n"
		"increment:\n"
		"    li t0, mydata\n"
		"    lw t1, [t0]\n"
		"    addi t1, t1, 1\n"
		"    sw [t0], ti\n"
		"    ret\n"
		"get_first_16:\n"
		"    ssi.c a0, noerror q0 mydata, 0\n"
		"    ret\n"
		"writable \n"
		"section \"data\"\n"
		"mydata:\n"
		"    d64 0xBAAD_F00D_DEAD_BEEF\n"
	;
	Arena data;
	arena_init(&data);

	lex(&out, &data, src);

	for_vec(LunaToken *cur, &out) {
		printf("TOK: ");
		if (*cur->ptr != '\n')
			printf("\"%.*s\"\t", cur->len, cur->ptr);

		switch(cur->kind) {
		case TOK_NEWLINE:
			printf("'\\n'");
			break;
		case TOK_STRING:
			printf("%s", cur->data);
			break;
		case TOK_IDENT:
			printf("(%s)", cur->data);
			break;
		case TOK_NUM:
			printf("(%zu %x %b)", cur->data, cur->data, cur->data);
			break;
		case TOK_OP:
			printf("[OP] %s", op_mnemonic[(usize) cur->data]);
			break;
		// This is a sin
		#define X(name, str) case TOK_KW_##name: \
					printf("["#name"]"); \
					break;
		KEYWORDS_LIST
		#undef X
		}
		printf("\n");
	}
	return 0;
}
