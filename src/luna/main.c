#include "common/vec.h"
#include "common/fs.h"
#include "common/str.h"
#include "common/util.h"

#include "luna.h"
#include "lex.h"
#include "parse.h"
#include "export.h"

#define REQUIRED_POSITIONAL_ARGS 1

// static void print_bin_fmt_a(u32 bits) {
//     u32 imm9 = bits >> 23;
//     u32 r3 = (bits >> 18) & 0b11111;
//     u32 r2 = (bits >> 13) & 0b11111;
//     u32 r1 = (bits >> 8) & 0b11111;
//     u32 opcode = (bits >> 2) & 0b111111;
//     u32 fmt = bits & 0b11;

//     printf("%09b %05b %05b %05b %06b %02b",
//         imm9,
//         r3, r2, r1, opcode, fmt
//     );
// }
//

int main(int argc, char** argv) {
    LunaInstance* luna = luna_new();

    SourceFileId mainfile;
    FsFile* outfile = nullptr;

    // tracks positional arguments
    usize position = 0;

    for_n (i, 1, argc) {
        string arg = string_wrap(argv[i]);

        // flags
        if (string_eq(arg, strlit("-h"))) {
            fprintf(stderr, "%s <input.s> [-o <out.bin>]\n", argv[0]);
            return 0;
        } else if (string_eq(arg, strlit("-o"))) {
            if (++i >= argc) {
                fprintf(stderr, "-o requires a filename\n");
                return 1;
            }
            outfile = fs_open(argv[i], true, true);
            continue;
        }

        // positional
        switch (position) {
        case 0:
            mainfile = luna_load_file(luna, arg);
            position++;
            break;
        default:
            fprintf(stderr, "Unexpected argument: "str_fmt"\n", str_arg(arg));
            return 1;
        }
    }

    if (position < REQUIRED_POSITIONAL_ARGS) {
        switch (position) {
        case 0: 
            fprintf(stderr, "Input filename required\n");
            break;
        default:
            UNREACHABLE;
        }
        return 1;
    }

    if (outfile == nullptr)
        outfile = fs_open("out.bin", true, true);

    Lexer lex = lexer_new(luna, mainfile);
    while (lexer_next_token(&lex)) {}
    Vec(Token) tokens = lexer_destroy_to_tokens(&lex);

    Parser p = parser_new(luna, tokens);
    Object o = parse_tokenbuf(&p);

    object_trace(&o);
    object_dbgprint(&o);

    string data = export_flat_binary(&o);

    fs_write(outfile, data.raw, data.len);
    fs_destroy(outfile);

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
