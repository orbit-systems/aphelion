#include "common/fs.h"
#include "common/str.h"
#include "common/vec.h"

#include "luna.h"

#include <stdio.h>

LunaInstance* luna_new() {
    LunaInstance* luna = malloc(sizeof(*luna));

    arena_init(&luna->permanent);

    const char* working_dir = fs_get_current_dir();
    luna->working_dir = arena_strdup(&luna->permanent, string_wrap(working_dir));
    luna->files = vec_new(SourceFile, 16);

    return luna;
}

SourceFileId luna_load_file(LunaInstance* luna, string path) {
    char buf_c[path.len + 1];
    memcpy(buf_c, path.raw, path.len);
    buf_c[path.len] = '\0';
    FsFile* file = fs_open(buf_c, false, false);

    if (file == nullptr) {
        printf("could not open file '%s'\n", buf_c);
        exit(0);
    }

    SourceFile srcfile = {
        .source = fs_read_entire(file, true),
        .path = arena_strdup(&luna->permanent, (string){
            .len = file->path.len,
            .raw = file->path.raw,
        })
    };

    vec_append(&luna->files, srcfile);

    return (SourceFileId){vec_len(luna->files) - 1};
}
