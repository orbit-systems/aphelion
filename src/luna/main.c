#include "common/type.h"

#include "luna.h"
#include "lex.h"

#include "stdio.h"

int main() {
    LunaInstance* luna = luna_new();

    SourceFileId mainfile = luna_load_file(luna, strlit(
        "tests/test.s"
    ));

    Lexer lex = lexer_new(luna, mainfile);

    while (lexer_next_token(&lex)) {}

    printf("lex successful\n");
}
