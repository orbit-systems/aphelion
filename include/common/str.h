#ifndef ORBIT_STRING_H
#define ORBIT_STRING_H

#include <stdlib.h>

// strings and string-related utils.

typedef struct string {
    char*  raw;
    size_t len;
} string;

#define NULL_STR ((string){nullptr, 0})
#define is_null_str(str) ((str).raw == nullptr)

#define str_fmt "%.*s"
#define str_arg(str) (int)(str).len, (str).raw

#define string_make(ptr, len) ((string){(ptr), (len)})
#define string_len(s) ((s).len)
#define string_raw(s) ((s).raw)
#define is_within(haystack, needle) (((haystack).raw <= (needle).raw) && ((haystack).raw + (haystack).len >= (needle).raw + (needle).len))
#define substring(str, start, end) ((string){(str).raw + (start), (end) - (start)})
#define substring_len(str, start, len) ((string){(str).raw + (start), (len)})
#define string_wrap(cstring) ((string){(char*)(cstring), strlen((cstring))})
#define strlit(cstring) ((string){(char*)cstring, sizeof(cstring)-1})

char*  clone_to_cstring(string str); // this allocates
void   printstr(string str);
string strprintf(char* format, ...);

string  string_alloc(size_t len);
#define string_free(str) free(str.raw)
string  string_clone(string str); // this allocates as well
string  string_concat(string a, string b); // allocates
void  string_concat_buf(string buf, string a, string b); // this does not

int  string_cmp(string a, string b);
bool string_eq(string a, string b);
bool string_ends_with(string source, string ending);

#endif // ORBIT_STRING_H
