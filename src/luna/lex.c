#include "common/util.h"

#include "aphelion.h"
#include "lex.h"
#include "luna.h"

const char* const inst_name_string[INST__COUNT] = {
    #define INST(variant, name) [INST_##variant] = name,
        INST_NAMES
    #undef INST
};

const char* const luna_keyword_string[] = {
    #define KW(variant, name) [TOK_KW_##variant] = name,
        LUNA_KEYWORDS
    #undef KW
};

Lexer lexer_new(LunaInstance* luna, SourceFileId file) {
    Lexer l = {
        .current_file = file,
        .cursor = 0,
        .tokens = vec_new(LunaToken, 256),
        .source = luna->files[file._].source,
        .luna = luna,
    };

    return l;
}

Vec(LunaToken) lexer_destroy_to_tokens(Lexer* l) {
    Vec(LunaToken) ts = l->tokens;
    *l = (Lexer){};
    return ts;
}

/// Moves a lexer `l`'s cursor one character forward.
static bool is_eof(Lexer* l) {
    return l->cursor >= l->source.len;
}

/// Move a lexer `l`'s cursor one character forward.
static void advance(Lexer* l) {
    l->cursor += 1;
}

/// Move a lexer `l`'s cursor `n` characters forward.
static void advance_n(Lexer* l, usize n) {
    l->cursor += n;
}

/// Check whether the character `n` bytes past
/// lexer `l`'s cursor matches the character `c`.
static bool peek_match(Lexer* l, usize n, char c) {
    if (l->cursor + n >= l->source.len) {
        return false;
    }
    return l->source.raw[l->cursor + n] == c;
}

/// Peek the character at a lexer's cursor.
/// lexer must not be at the end of the string.
static char current(Lexer* l) {
    return l->source.raw[l->cursor];
} 

/// Advance a lexer's cursor past whitespace characters, 
/// or until the end of the source text
static void skip_whitespace(Lexer* l) {
    while (!is_eof(l)) {
        switch (current(l)) {
        case '\v':
        case '\t':
        case '\r':
        case ' ':
            advance(l);
            continue;
        default:
            return;
        }
    }
}

/// Add a token to lexer `l`'s token buffer.
/// Returns a pointer to the token added that will be
/// valid until the next `add_token` call.
static LunaToken* add_token(Lexer* l, LunaTokenKind k) {
    LunaToken t = {
        .kind = k,
        .subkind = 0,
        .raw = (i64)&l->source.raw[l->cursor],
    };

    vec_append(&l->tokens, t);

    // return &l->tokens[vec_len(l->tokens) - 1];
    return nullptr;
}

/// Add an EOF token to lexer `l`'s token buffer.
static void add_eof_token(Lexer* l) {
    LunaToken t = {
        .kind = TOK_EOF,
        .subkind = 0,
        .raw = (i64)&l->source.raw[l->source.len - 1],
    };

    vec_append(&l->tokens, t);
}

/// Check if the character `c` can be in the middle of an identifier.
static bool is_ident_middle(char c) {
    switch (c) {
    case '.':
    case '_':
    case 'a' ... 'z':
    case 'A' ... 'Z':
    case '0' ... '9':
        return true;
    default:
        return false;
    }
}

/// Check if the character `c` can be in the middle of a numeric constant.
static bool is_numeric_middle(char c) {
    switch (c) {
    case '_':
    case 'a' ... 'z':
    case 'A' ... 'Z':
    case '0' ... '9':
        return true;
    default:
        return false;
    }
}

static void categorize_ident(const char* start, usize length, LunaToken* t) {
    string span = {
        .len = length,
        .raw = (char*) start,
    };

    // TODO this is... bad. replace this with a hashmap.

    #define GPR(variant, name) if (string_eq(span, strlit(name))) { t->kind = TOK_GPR; t->subkind = GPR_##variant; return; }
        APHEL_GPRS
    #undef GPR

    #define CTRL(variant, name) if (string_eq(span, strlit(name))) { t->kind = TOK_CTRL; t->subkind = CTRL_##variant; return; }
        APHEL_CTRLS
    #undef CTRL

    #define INST(variant, name) if (string_eq(span, strlit(name))) { t->kind = TOK_INST; t->subkind = INST_##variant; return; }
        INST_NAMES
    #undef INST
    
    #define KW(variant, name) if (string_eq(span, strlit(name))) { t->kind = TOK_KW_##variant; return; }
        LUNA_KEYWORDS
    #undef CTRL

    t->kind = TOK_IDENT;
}

/// Scan lexer `l`'s source text for the next token.
/// \return Whether a new token can be generated from a subsequent call.
bool lexer_next_token(Lexer* l) {
    skip_whitespace(l);
    if (is_eof(l)) {
        add_eof_token(l);
        return false;
    }

    switch (current(l)) {
    // misc symbols
    case '\n':
    case '(':
    case ')':
    case '[':
    case ']':
    case '{':
    case '}':
    case ',': 
    case ':':
    case '=':
    case '+':
    case '-':
    case '*':
    case '/':
    case '%':
    case '&':
    case '|':
    case '~':
        add_token(l, current(l));
        advance(l);
        break;

    // string literals
    // NOTE: may contain incorrect escape characters.
    //       we'll emit errors about that when its value is calculated.
    case '\"': {
        LunaToken* t = add_token(l, TOK_STR_LIT);
        bool escape = false;

        advance(l);
        if (is_eof(l)) {
            TODO("unclosed string literal");
        }

        while (!(current(l) == '\"' && !escape)) {
            if (escape) {
                escape = false;
            } else {
                escape = current(l) == '\\';
            }
            advance(l);

            if (is_eof(l)) {
                TODO("unclosed string literal");
            }
        }
        advance(l);
        break;
    }

    // char literals
    // NOTE: may contain incorrect escape characters.
    //       we'll emit errors about that when its value is calculated.
    case '\'': {
        LunaToken* t = add_token(l, TOK_STR_LIT);
        bool escape = false;

        advance(l);
        if (is_eof(l)) {
            TODO("unclosed string literal");
        }

        while (!(current(l) == '\'' && !escape)) {
            if (escape) {
                escape = false;
            } else {
                escape = current(l) == '\\';
            }
            advance(l);

            if (is_eof(l)) {
                TODO("unclosed string literal");
            }
        }
        advance(l);
        break;
    }

    // numeric literals
    // NOTE: digits in here might not be in the correct.
    //       we'll emit errors about that when its value is calculated.
    case '0' ... '9':
        add_token(l, TOK_NUM_LIT);
        // scan forward
        while (!is_eof(l) && is_numeric_middle(current(l))) {
            advance(l);
        }
        break;

    // identifiers/keywords
    case '.':
    case '_':
    case 'a' ... 'z':
    case 'A' ... 'Z': {
        LunaToken* t = add_token(l, TOK_IDENT);
        // scan forward
        usize start = l->cursor;
        while (!is_eof(l) && is_ident_middle(current(l))) {
            advance(l);
        }
        usize length = l->cursor - start;

        // categorize_ident(token_start(*t), length, t);
    } break;
    default:
        TODO("encountered unhandled character '%c'", current(l));
    }

    return true;
}
