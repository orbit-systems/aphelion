// ╭──────────╮
// │ aas v0.1 │ (C) 2023 sandwichman
// ╰──────────╯
// Aphelion Assembler

// usage: aas (path) [flags]
 
// -debug               print debug info
// -out:[path]          set output path
// -preprocess          only invoke preprocessor - expand macros, etc.
// -ignore-ext          ignore file extension
// -help                display this text

// TODO finish lexer, start parser, preprocessor, binary embedder

package aas

import "core:os"
import "core:strings"
import "core:fmt"
import "core:path/slashpath"

main :: proc() {

    // schlorp arguments
    {
        if len(os.args) < 2 {
            print_help()
            os.exit(0)
        }

        parsed_args : [dynamic]cmd_arg
        defer delete(parsed_args)

        for argument in os.args[1:] {
            split_arg := strings.split(argument, ":")
            if len(split_arg) == 1 {
                append(&parsed_args, cmd_arg{argument, ""})
            } else {
                append(&parsed_args, cmd_arg{split_arg[0], split_arg[1]})
            }
        }

        for argument, index in parsed_args {
            switch argument.key {
            case "-help":
                print_help()
                os.exit(0)
            case "-debug":
                print_dbg = true
            case "-ignore-ext":
                ignore_ext = true
            case "-out":
                outpath = argument.val
                outpath_loaded = true
            case: // default
                if index == 0 && argument.key[0] != '-' {
                    inpath = argument.key
                    continue
                }
                die("err: invalid argument \"%s\"\n", argument.key)
            }
        }
    }
    dbg("arguments loaded\n")

    // schlorp assembly
    raw, ok := os.read_entire_file(inpath)
    if !ok {
        die("err: cannot read file at \"%s\"\n", inpath)
    }
    if slashpath.ext(inpath) != ".aphel" && !ignore_ext {
        die("err: \"%s\" is not of type \".aphel\", check file extension\n", inpath)
    }
    dbg("file found at \"%s\"\n", inpath)
    raw_asm := string(raw)

    // tokenize
    token_chain : [dynamic]aphel_token
    defer delete(token_chain)
    tokenize(raw_asm, &token_chain)
    dbg("%d tokens indexed\n", len(token_chain))

    // debug display token chain
    {
        dbg("-------------------------------------\n")
        for i in token_chain {
            dbg(i.value)
            dbg(" ")
        }
        dbg("\n-------------------------------------\n")

        max_len := 0
        for i in token_chain {      // determine maximum token length for nice printing
            max_len = max(len(i.value), max_len)
        }

        for i in token_chain {
            if i.value == "\n" {
                dbg(strings.repeat(" ", max_len))
                dbg("  %s\n", i.kind)
                continue
            }
            dbg("%s", i.value)
            dbg(strings.repeat(" ", max_len-len(i.value)))
            dbg("  %s\n", i.kind)
            
        }
        dbg("-------------------------------------\n")
    }

    
}

cmd_arg :: struct {
    key : string,
    val : string,
}

// init vars
inpath          : string
outpath         : string
print_dbg       := false
ignore_ext      := false
outpath_loaded  := false