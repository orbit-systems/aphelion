#ifndef ORBIT_FS_H
#define ORBIT_FS_H

#define _XOPEN_SOURCE 700

#include "type.h"
#include "str.h"
#include "vec.h"
#include "util.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined(OS_WINDOWS)
    #define WIN32_LEAN_AND_MEAN
    #include <windows.h>
#elif defined(OS_LINUX)
    #include <limits.h>
#else
    #error "Unsupported OS. Go harass sandwichman about it."
#endif

typedef struct FsPath {
    u16 len;
    char raw[PATH_MAX];
} FsPath;

typedef struct FsFile {
    isize handle;
    usize id;
    usize size;
    // units unspecified, but greater means more recent
    usize last_modified;
    FsPath path;
} FsFile;

#define fs_strref_from_path(pathptr) (string){.len = (pathptr)->len, .raw = (pathptr)->raw}

bool fs_real_path(const char* path, FsPath* out);
FsFile* fs_open(const char* path, bool create, bool overwrite);
usize fs_read(FsFile* f, void* buf, usize len);
string fs_read_entire(FsFile* f, bool nullterm);
usize fs_write(FsFile* f, void* buf, usize len);
void fs_close(FsFile* f);
void fs_destroy(FsFile* f);

char* fs_get_current_dir();
bool fs_set_current_dir(const char* dir);

// returns contents. if contents == nullptr, return a newly allocated vec.
Vec(string) fs_dir_contents(const char* path, Vec(string)* contents);

#endif // ORBIT_NEWFS_H
