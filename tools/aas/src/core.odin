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

// TODO test    : lexer
// TODO finish  : parser
// TODO start   : embedder, preprocessor

package aas

import "core:os"
import "core:time"
import "core:strings"
import "core:fmt"
import "core:path/slashpath"

main :: proc() {

    timer : time.Stopwatch
    time.stopwatch_start(&timer)
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
                die("ERR: invalid argument \"%s\"\n", argument.key)
            }
        }
    }
    dbg("arguments loaded\n")



    // schlorp assembly
    raw, ok := os.read_entire_file(inpath)
    if !ok {
        die("ERR: cannot read file at \"%s\"\n", inpath)
    }
    if slashpath.ext(inpath) != ".aphel" && !ignore_ext {
        die("ERR: \"%s\" is not of type \".aphel\", check file extension\n", inpath)
    }
    dbg("file found at \"%s\"\n", inpath)
    raw_asm := string(raw)



    // tokenize
    dbg("tokenizing...")
    token_chain : [dynamic]btoken
    defer delete(token_chain)
    tokenize(raw_asm, &token_chain)
    dbgokay()
    dbg(" (%d tokens indexed)\n", len(token_chain))

    // debug display token chain
    display_more := len(token_chain) <= 30 && print_dbg // && false
    if display_more {       // dont clutter the terminal
        // dbg("-------------------------------------\n")
        // for i in token_chain {
        //     dbg(i.value)
        //     dbg(" ")
        // }
        // dbg("\n-------------------------------------\n")

        max_len := 0
        for i in token_chain {       // determine maximum token length for nice printing
            max_len = max(len(i.value), max_len)
        }

        for i in token_chain {
            dbg("\t")
            if i.value == "\n" {
                dbg(strings.repeat(" ", max_len))
                dbg("  %s\n", i.kind)
                continue
            }
            dbg("%s", i.value)
            dbg(strings.repeat(" ", max_len-len(i.value)))
            dbg("  %s\n", i.kind)
            
        }
        // dbg("-------------------------------------\n")
    }



    // parse
    dbg("parsing...")
    statement_chain : [dynamic]statement
    defer delete(statement_chain)
    construct_statement_chain(&statement_chain, &token_chain)
    dbgokay()
    dbg(" (%d statements indexed)\n", len(statement_chain))

    // debug display statement chain
    if display_more {       // dont clutter the terminal

        for i in statement_chain {

            if i.kind == statement_kind.Directive {
                set_style(ANSI.FG_Yellow)
            }
            if i.kind == statement_kind.Instruction {
                set_style(ANSI.FG_Red)
            }
            if i.kind != statement_kind.Label {
                dbg("\t\t")
            }

            dbg("%s ", i.name)
            set_style(ANSI.Reset)

            if i.kind == statement_kind.Label {
                dbg("\n")
                continue
            }

            for arg, index in i.args {
                if arg == (argument{}) {
                    dbg("_")
                } else if arg.kind == argument_kind.Integer {
                    dbg("%d", arg.value_int)
                } else {
                    dbg(arg.value_str)
                }

                if index != len(i.args)-1 {
                    dbg(", ")
                }

            }
            dbg("\n")
            
        }
        // dbg("-------------------------------------\n")
    }




    time.stopwatch_stop(&timer)
    dbg("assembly took %f seconds\n", time.duration_seconds(time.stopwatch_duration(timer)))

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