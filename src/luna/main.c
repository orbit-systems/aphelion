#include "common/vec.h"

#include "luna.h"
#include "lex.h"
#include "parse.h"


int main() {
    LunaInstance* luna = luna_new();

    SourceFileId mainfile = luna_load_file(luna, strlit(
        "tests/basic.s"
    ));

    Lexer lex = lexer_new(luna, mainfile);
    while (lexer_next_token(&lex)) {}
    Vec(Token) tokens = lexer_destroy_to_tokens(&lex);

    Parser p = parser_new(luna, tokens);
    parse_file(&p);
}
