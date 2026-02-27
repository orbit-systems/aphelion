#ifndef LUNA_H
#define LUNA_H

#include "common/type.h"
#include "common/str.h"
#include "common/vec.h"
#include "common/arena.h"

// TODO: place this in common somewhere
#if __STDC_HOSTED__ && (defined(__x86_64__) || defined(__aarch64__))
    /// Pointers are sign-extended from 48-bits.
    #define HOST_POINTER_48_BITS
#endif

/// An opaque handle used to refer to a source file with a LunaInstance.
typedef struct SourceFileId {u32 _;} SourceFileId;

/// A source file the assembler is analyzing.
typedef struct SourceFile {
    /// The absolute path to the source file.
    string path;
    /// The full source text.
    string source;
} SourceFile;

/// An instance of the Luna assembler.
typedef struct LunaInstance {
    /// Full path of the current working directory.
    string working_dir;

    /// All the source files being processed by the assembler at the moment.
    Vec(SourceFile) files;

    /// Storage for data that persists across the entire compilation.
    /// Examples include processed string literals, file paths, etc.
    Arena permanent;
} LunaInstance;

/// Create a new luna instance.
LunaInstance* luna_new();

// Load a file into LunaInstance `luna`.
SourceFileId luna_load_file(LunaInstance* luna, string path);


#endif // LUNA_H
