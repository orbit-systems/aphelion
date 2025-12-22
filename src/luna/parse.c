#include "parse.h"
#include "common/util.h"
#include "lex.h"
#include "luna.h"
#include "reporting.h"

static void parse_instruction(Parser* p);

static SourceFileId find_source_from_ptr(Vec(SourceFile) files, const char* s) {
    for_n(i, 0, vec_len(files)) {
        string source = files[i].source;
        if (source.raw <= s && s <= source.raw + source.len) {
            return (SourceFileId){i};
        }
    }
    CRASH("cannot find corresponding source file");
}

#define parse_error(p, token_index, msg, ...) \
    parse_error__internal(p, token_index, strprintf(msg, __VA_ARGS__))

static void parse_error__internal(Parser* p, usize token_index, string msg) {
    const char* current_start = token_start(p->tokens[token_index]);

    SourceFileId fileid = find_source_from_ptr(
        p->luna->files, 
        current_start
    );
    SourceFile file = p->luna->files[fileid._];

    Report* r = report_new(REPORT_ERROR, msg, &p->luna->files);
    
    usize span_start = current_start - file.source.raw;
    usize span_end = span_start + 1;

    report_add_label(r, REPORT_LABEL_PRIMARY, fileid, span_start, span_end, strlit(""));

    report_render(stderr, r);
    report_destroy(r);

    exit(1);
}

static inline void advance(Parser* p) {
    p->cursor += 1;
    p->current = p->tokens[p->cursor];
}

static inline void expect_kind(Parser* p, TokenKind kind) {
    if_unlikely (p->current.kind != kind) {
        // TODO("expected token kind %d", kind);
        parse_error(p, p->cursor, "expected %s, got %s", 
            token_kind_name[kind],
            token_kind_name[p->current.kind]
        );
    }
}

static inline void expect_advance(Parser* p, TokenKind kind) {
    expect_kind(p, kind);
    advance(p);
}

static string get_strlit_contents(Parser* p) {
    Arena* arena = &p->luna->permanent;

    Token t = p->current;

    if (t.subkind == TOK_STR_HAS_ESCAPE) {
        TODO("handle escape sequences in string");
    } else {
        const char* content_start = &token_start(t)[1];
        const char* content_end = strchr(content_start, '\"');

        return (string){
            .raw = (char*) content_start,
            .len = content_end - content_start,
        };
    }
}

static bool parse_sec_element(Parser* p) {
    Token t = p->current;

    switch (t.kind) {
    case TOK_NEWLINE:
        // try again.
        advance(p);
        return parse_sec_element(p);
    case TOK_INST:
        parse_instruction(p);
        return true;

    default:
        return false;
    }
}

static void parse_section(Parser* p, ApoSectionFlags flags) {
    Arena* arena = &p->luna->permanent;

    expect_advance(p, TOK_KW_SECTION);
    expect_kind(p, TOK_STR_LIT);
    string section_name = get_strlit_contents(p);
    advance(p);
    expect_advance(p, TOK_NEWLINE);

    // create a new section.
    Section* section = arena_alloc(arena, sizeof(Section), alignof(Section));
    *section = (Section){
        .name = section_name.raw,
        .name_len = section_name.len,
        .alignment_p2 = 0,
        .flags = flags,
        .map_address = p->address,
        .bytes = vec_new(u8, 128),
    };

    vec_append(&p->sections, section);
    p->current_section = section;

    while (parse_sec_element(p)) {}
}

void parse_section_or_group(Parser* p) {
    // collect flags
    ApoSectionFlags flags = 0;
    while (true) {
        Token t = p->current;
        switch (p->current.kind) {
        case TOK_KW_UNMAPPED:
            flags |= APO_SECFL_UNMAPPED;
            advance(p);
            break;
        case TOK_KW_WRITABLE:
            flags |= APO_SECFL_WRITABLE;
            advance(p);
            break;
        case TOK_KW_EXECUTABLE:
            flags |= APO_SECFL_EXECUTABLE;
            advance(p);
            break;
        case TOK_KW_BLANK:
            flags |= APO_SECFL_BLANK;
            advance(p);
            break;
        case TOK_KW_PINNED:
            flags |= APO_SECFL_PINNED;
            advance(p);
            break;
        case TOK_KW_COMMON:
            flags |= APO_SECFL_COMMON;
            advance(p);
            break;
        case TOK_KW_NONVOLATILE:
            flags |= APO_SECFL_NONVOLATILE;
            advance(p);
            break;
        case TOK_KW_CONCATENATE:
            flags |= APO_SECFL_CONCATENATE;
            advance(p);
            break;
        case TOK_KW_SECTION:
            parse_section(p, flags);
            return;
        default:
            TODO("expected 'section' or 'group'");
        }
    }
}

Parser parser_new(LunaInstance* luna, Vec(Token) tokens) {
    Parser p = {
        .tokens = tokens,
        .cursor = 0,
        .current = tokens[0],
        .sections = vec_new(Section*, 16),
        .current_section = nullptr,
        .current_section_group = nullptr,
        .luna = luna,
        .address = 0,
    };

    return p;
}

void parse_file(Parser* p) {
    switch (p->current.kind) {
    case TOK_KW_UNMAPPED:
    case TOK_KW_WRITABLE:
    case TOK_KW_EXECUTABLE:
    case TOK_KW_BLANK:
    case TOK_KW_PINNED:
    case TOK_KW_COMMON:
    case TOK_KW_NONVOLATILE:
    case TOK_KW_CONCATENATE:
        parse_section_or_group(p);
        break;
    case TOK_KW_SECTION:
        parse_section(p, 0);
        break;
    }
}


typedef enum : u8 {
    OPERAND_GPR,
    OPERAND_CTRL,
    OPERAND_EXPR,
    OPERAND_RELOC,
    OPERAND_MEM,
} OperandKind;

typedef struct {
    OperandKind kind;
    union {
        AphelGpr gpr;
        AphelCtrl ctrl;
        u64 expr;
        struct {
            AphelGpr gpr1;
            AphelGpr gpr2;
            u16 expr;
        } mem;
    };
} Operand;

static Operand parse_operand(Parser* p) {
    Token t = p->current;
    switch (t.kind) {
    case TOK_OPEN_BRACKET:
        // memory operand.
        TODO("parse memory operand");
    case TOK_GPR:
        advance(p);
        return (Operand){
            .kind = OPERAND_GPR,
            .gpr = t.subkind,
        };
    case TOK_CTRL:
        advance(p);
        return (Operand){
            .kind = OPERAND_CTRL,
            .gpr = t.subkind,
        };
    default:
        TODO("expected operand");
    }
}

static void parse_instruction(Parser* p) {
    Token inst = p->current;
    expect_advance(p, TOK_INST);

    // get operand list
    Operand operands[8] = {};
    usize operands_len = 0;

    while (p->current.kind != TOK_NEWLINE) {
        
    }
}
