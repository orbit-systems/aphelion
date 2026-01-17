#include "common/vec.h"
#include "common/fs.h"

#include "luna.h"
#include "lex.h"
#include "parse.h"
#include "export.h"

static void print_bin_fmt_a(u32 bits) {
    u32 imm9 = bits >> 23;
    u32 r3 = (bits >> 18) & 0b11111;
    u32 r2 = (bits >> 13) & 0b11111;
    u32 r1 = (bits >> 8) & 0b11111;
    u32 opcode = (bits >> 2) & 0b111111;
    u32 fmt = bits & 0b11;

    printf("%09b %05b %05b %05b %06b %02b",
        imm9,
        r3, r2, r1, opcode, fmt
    );
}

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

    // Vec(u8) bin = export_flat_binary(p.sections, vec_len(p.sections));

    // for_n(i, 0, vec_len(bin) / 4) {
    //     // u8 byte = bin[i];
    //     u32 hword = *(u32*)&bin[i * 4];

    //     print_bin_fmt_a(hword);

    //     printf("\n");
    // }

    // FsFile* out = fs_open(
    //     "output.bin",
    //     true,
    //     true
    // );

    // int written = fs_write(out, bin, vec_len(bin));

    // // printf("written %d\n", written);

    // fs_destroy(out);

}
