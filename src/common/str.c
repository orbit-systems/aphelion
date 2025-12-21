#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include "common/str.h"

string strprintf(char* format, ...) {
    string c = NULL_STR;
    va_list a;
    va_start(a, format);
    va_list b;
    va_copy(b, a);
    size_t bufferlen = 1 + vsnprintf("", 0, format, a);
    c = string_alloc(bufferlen);
    vsnprintf(c.raw, c.len, format, b);
    c.len--;
    va_end(a);
    va_end(b);
    return c;
}

string string_concat(string a, string b) {
    string c = string_alloc(a.len + b.len);
    string_concat_buf(c, a, b);
    return c;
}

void string_concat_buf(string buf, string a, string b) {
    for (size_t i = 0; i < a.len; ++i) {
        buf.raw[i] = a.raw[i];
    }
    for (size_t i = 0; i < b.len; ++i) {
        buf.raw[i + a.len] = b.raw[i];
    }
}

bool string_ends_with(string source, string ending) {
    if (source.len < ending.len) return false;

    return string_eq(substring_len(source, source.len-ending.len, ending.len), ending);
}

string string_alloc(size_t len) {
    char* raw = malloc(len);

    if (raw == nullptr) return NULL_STR;

    memset(raw, '\0', len);

    raw[len] = '\0';

    return (string){raw, len};

}

int string_cmp(string a, string b) {
    // copied from odin's implementation lmfao
    int res = memcmp(a.raw, b.raw, a.len < b.len ? a.len : b.len);
    if (res == 0 && a.len != b.len) return a.len <= b.len ? -1 : 1;
    else if (a.len == 0 && b.len == 0) return 0;
    return res;
}

bool string_eq(string a, string b) {
    if (a.len != b.len) return false;
    for (size_t i = 0; i < a.len; ++i) {
        if (a.raw[i] != b.raw[i]) return false;
    }
    return true;
}

char* clone_to_cstring(string str) {
    if (is_null_str(str)) return "";

    char* cstr = malloc(str.len + 1);
    if (cstr == nullptr) return nullptr;
    memcpy(cstr, str.raw, str.len);
    cstr[str.len] = '\0';
    return cstr;
}

string string_clone(string str) {
    string new_str = string_alloc(str.len);
    if (memmove(new_str.raw, str.raw, str.len) != new_str.raw) return NULL_STR;
    return new_str;
}

void printn(char* text, size_t len) {
    size_t c = 0;
    while (c < len && text[c] != '\0')
        putchar(text[c++]);
}

void printstr(string str) {
    printn(str.raw, str.len);
}
