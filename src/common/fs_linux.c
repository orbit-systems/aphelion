#include <string.h>
#define _GNU_SOURCE

#include "common/fs.h"

#ifdef OS_LINUX

#include <dirent.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>

bool fs_real_path(const char* path, FsPath* out) {
    if (!realpath(path, out->raw)) return true;
    out->len = strlen(out->raw);
    return true;
}

FsFile* fs_open(const char* path, bool create, bool overwrite) {
    FsFile* f = malloc(sizeof(FsFile));
    if (create) {
        if (overwrite) {
            f->handle = open(path, O_WRONLY | O_CREAT);
        } else {
            f->handle = open(path, O_WRONLY);
        }
    } else {
        f->handle = open(path, O_RDONLY);
    }
    
    if (f->handle == -1) return nullptr;

    struct stat info;
    fstat(f->handle, &info);
    f->id = info.st_ino;
    f->size = info.st_size;
    // convert to microseconds, that's easier and more reliable to deal with in 64 bits
    f->last_modified = ((usize)info.st_mtim.tv_sec * 1000000) + (usize)info.st_mtim.tv_nsec / 1000;
    fs_real_path(path, &f->path);
    return f;
}

usize fs_write(FsFile* f, void* buf, usize len) {
    isize num_written = write(f->handle, buf, len);
    if (num_written == -1) return 0;
    return (usize) num_written;
}

usize fs_read(FsFile* f, void* buf, usize len) {
    isize num_read = read(f->handle, buf, len);
    if (num_read == -1) return 0;
    return (usize) num_read;
}

string fs_read_entire(FsFile* f) {
    string s = string_alloc(f->size);
    read(f->handle, s.raw, s.len);
    return s;
}
void fs_close(FsFile* f) {
    close(f->handle);
    f->handle = -1;
}

void fs_destroy(FsFile* f) {
    if (f->handle != 0 && f->handle != -1) {
        fs_close(f);
    }
    free(f);
}

char* fs_get_current_dir() {
    // char* current_dir = get_current_dir_name();
    usize cap = 256;
    char* buffer = malloc(cap);
    while (!getcwd(buffer, cap)) {
        cap *= 2;
        buffer = realloc(buffer, cap);
    }

    char* cwd = malloc(strlen(buffer) + 1);
    strcpy(cwd, buffer);
    free(buffer);

    return cwd;
}

bool fs_set_current_dir(const char* dir) {
    return chdir(dir) == 0;
}

// returns contents. if contents == nullptr, return a newly allocated vec.
Vec(string) fs_dir_contents(const char* path, Vec(string)* _contents) {
    DIR* d = opendir(path);

    Vec(string) contents = {0};
    if (_contents == nullptr) {
        contents = vec_new(string, 16);
    } else {
        contents = *_contents;
    }
    
    struct dirent* dirent;
    while ((dirent = readdir(d)) != nullptr) {
        if (strcmp(dirent->d_name, ".") == 0) continue;
        if (strcmp(dirent->d_name, "..") == 0) continue;
        vec_append(&contents, string_clone(string_wrap(dirent->d_name)));
    }

    closedir(d);
    return contents;
}

#endif
