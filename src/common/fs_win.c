#include "common/fs.h"

#ifdef OS_WINDOWS

bool fs_real_path(const char* path, FsPath* out) {
    if (GetFullPathNameA(path, PATH_MAX, out->raw, nullptr) == 0) {
        return false;
    }
    out->len = strlen(out->raw);

    // fuck backslash all my homies hate backslash
    for_n(i, 0, out->len) {
        if (out->raw[i] == '\\') {
            out->raw[i] = '/';
        }
    }

    return true;
}

FsFile* fs_open(const char* path, bool create, bool overwrite) {
    FsFile* f = malloc(sizeof(FsFile));
    if (create) {
        f->handle = (isize) CreateFileA(
            path, GENERIC_WRITE, FILE_SHARE_WRITE, nullptr, 
            overwrite ? CREATE_ALWAYS : CREATE_NEW,
            0, 0
        );
    } else {
        f->handle = (isize) CreateFileA(
            path, GENERIC_READ, FILE_SHARE_READ, nullptr, 
            overwrite ? OPEN_ALWAYS : OPEN_EXISTING,
            0, 0
        );
    }

    if ((void*) f->handle == INVALID_HANDLE_VALUE) {
        return nullptr;
    }

    BY_HANDLE_FILE_INFORMATION info;
    GetFileInformationByHandle((HANDLE)f->handle, &info);

    ULARGE_INTEGER windows_why_are_u_say_zis;

    windows_why_are_u_say_zis.HighPart = info.nFileIndexHigh;
    windows_why_are_u_say_zis.LowPart  = info.nFileIndexLow;
    f->id = (usize) windows_why_are_u_say_zis.QuadPart;

    windows_why_are_u_say_zis.HighPart = info.ftLastWriteTime.dwHighDateTime;
    windows_why_are_u_say_zis.LowPart  = info.ftLastWriteTime.dwLowDateTime;
    f->last_modified = (usize) windows_why_are_u_say_zis.QuadPart;

    windows_why_are_u_say_zis.HighPart = info.nFileSizeHigh;
    windows_why_are_u_say_zis.LowPart  = info.nFileSizeLow;
    f->size = (usize) windows_why_are_u_say_zis.QuadPart;

    fs_real_path(path, &f->path);

    return f;
}

// returns how much was /actually/ read.
usize fs_read(FsFile* f, void* buf, usize len) {
    DWORD num_read = 0;
    ReadFile((HANDLE)f->handle, buf, len, &num_read, nullptr);
    return num_read;
}

string fs_read_entire(FsFile* f) {
    string buf = string_alloc(f->size);
    DWORD num_read;
    ReadFile((HANDLE)f->handle, buf.raw, buf.len, &num_read, nullptr);
    return buf;
}

void fs_close(FsFile* f) {
    CloseHandle((HANDLE)f->handle);
    f->handle = (isize)INVALID_HANDLE_VALUE;
}

void fs_destroy(FsFile* f) {
    if (f->handle != 0 && f->handle != (isize)INVALID_HANDLE_VALUE) {
        fs_close(f);
    }
    free(f);
}

Vec(string) fs_dir_contents(const char* path, Vec(string)* _contents) {

    Vec(string) contents = {0};

    usize path_len = strlen(path);
    char* search_path = malloc(path_len + 3);
    memcpy(search_path, path, path_len);
    search_path[path_len] = '/';
    search_path[path_len + 1] = '*';
    search_path[path_len + 2] = 0;

    if (_contents == nullptr) {
        contents = vec_new(string, 16);
    } else {
        contents = *_contents;
    }

    WIN32_FIND_DATAA data;
    HANDLE search_handle = FindFirstFileA(search_path, &data);
    if (search_handle == INVALID_HANDLE_VALUE) {
        return contents;
    }

    do {
        if (strcmp(data.cFileName, ".") == 0) continue;
        if (strcmp(data.cFileName, "..") == 0) continue;
        vec_append(&contents, string_clone(string_wrap(data.cFileName)));

    } while (FindNextFileA(search_handle, &data));

    FindClose(search_handle);
    free(search_path);
    return contents;
}

char* fs_get_current_dir() {
    usize required = GetCurrentDirectoryA(0, nullptr);
    char* current_dir = malloc(required + 1);
    current_dir[required] = 0;
    GetCurrentDirectoryA(required, current_dir);
    for_n(i, 0, required) {
        if (current_dir[i] == '\\') {
            current_dir[i] = '/';
        }
    }
    return current_dir;
}

bool fs_set_current_dir(const char* dir) {
    return SetCurrentDirectoryA(dir);
}

#endif
