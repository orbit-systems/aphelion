package aas

// ╭──────────╮
// │ aas v0.1 │ by sandwichman
// ╰──────────╯
// Aphelion Assembler

// usage: aas (path) [flags]
 
// -debug               print debug info
// -out:[path]          set output path
// -no-color            disable output coloring
// -prep                only invoke preprocessor - expand macros, etc.
// -ignore-ext          ignore file extension
// -help                display this text

// todo test    : lexer, parser, embedder
// todo finish  : 
// todo start   : preprocessor
// * ideas
// * cache token chain / statment chain for fast recompilation - im not smart enough to implement this yet

import "core:os"
import "core:time"
import "core:strings"
import "core:fmt"
import "core:path/slashpath"
import "core:path/filepath"

main :: proc() {

    overall_timer : time.Stopwatch
    individual_timer : time.Stopwatch
    time.stopwatch_start(&overall_timer)
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
                flag_print_dbg = true
            case "-ignore-ext":
                flag_ignore_ext = true
            case "-no-color":
                flag_no_color = true
            case "-out":
                outpath = argument.val
                flag_outpath_loaded = true
            case "-prep":
                flag_prep_only = true
            case: // default
                if index == 0 && argument.key[0] != '-' {
                    inpath = argument.key
                    continue
                }
                die("ERR: invalid argument \"%s\"\n", argument.key)
            }
        }
    }
    //dbg("arguments loaded\n")
    if !flag_outpath_loaded {
        outpath = replace_ext(inpath, ".bin")
    }



    // schlorp assembly
    raw, readok := os.read_entire_file(inpath)
    if !readok {
        die("ERR: cannot read file at \"%s\"\n", inpath)
    }
    if slashpath.ext(inpath) != ".aphel" && !flag_ignore_ext {
        die("ERR: \"%s\" is not of type \".aphel\", check file extension\n", inpath)
    }
    //dbg("file found at \"%s\"\n", inpath)
    raw_asm := string(raw)



    // tokenize
    dbg("tokenizing...        ")
    token_chain : [dynamic]btoken
    //defer delete(token_chain)
    time.stopwatch_reset(&individual_timer)
    time.stopwatch_start(&individual_timer)
    tokenize(raw_asm, &token_chain)
    time.stopwatch_stop(&individual_timer)
    dbgokay()
    dbg(" %fs", time.duration_seconds(time.stopwatch_duration(individual_timer)))
    set_style(ANSI.Dim)
    dbg(" (%d tokens indexed)\n", len(token_chain))
    set_style(ANSI.Reset)

    // preprocess
    // dbg("preprocessing...     ")
    // time.stopwatch_reset(&individual_timer)
    // time.stopwatch_start(&individual_timer)
    // preprocess(&token_chain)
    // time.stopwatch_stop(&individual_timer)
    // dbgokay()
    // dbg(" %fs", time.duration_seconds(time.stopwatch_duration(individual_timer)))
    // set_style(ANSI.Dim)
    // dbg(" (%d tokens left)\n", len(token_chain))
    // set_style(ANSI.Reset)

    // debug display token chain
    display_more := flag_prep_only && flag_print_dbg
    if display_more {
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

    if flag_prep_only {
        delete(token_chain)
        return
    }

    dbg("building chain...    ")
    statement_chain : [dynamic]statement
    defer delete(statement_chain)
    time.stopwatch_reset(&individual_timer)
    time.stopwatch_start(&individual_timer)
    construct_stmt_chain(&statement_chain, &token_chain)
    time.stopwatch_stop(&individual_timer)
    dbgokay()
    dbg(" %fs", time.duration_seconds(time.stopwatch_duration(individual_timer)))
    set_style(ANSI.Dim)
    dbg(" (%d statements indexed)\n", len(statement_chain))
    set_style(ANSI.Reset)

    delete(token_chain)
    
    dbg("checking...          ")
    time.stopwatch_reset(&individual_timer)
    time.stopwatch_start(&individual_timer)
    check_stmt_chain(&statement_chain)
    time.stopwatch_stop(&individual_timer)
    dbgokay()
    dbg(" %fs", time.duration_seconds(time.stopwatch_duration(individual_timer)))
    set_style(ANSI.Dim)
    dbg(" (%d statements checked)\n", len(statement_chain))
    set_style(ANSI.Reset)

    dbg("tracing image...     ")
    time.stopwatch_reset(&individual_timer)
    time.stopwatch_start(&individual_timer)
    predicted_len := trace(&statement_chain)
    time.stopwatch_stop(&individual_timer)
    dbgokay()
    dbg(" %fs", time.duration_seconds(time.stopwatch_duration(individual_timer)))
    set_style(ANSI.Dim)
    dbg(" (%d statements traced)\n", len(statement_chain))
    set_style(ANSI.Reset)

    dbg("resolving labels...  ")
    time.stopwatch_reset(&individual_timer)
    time.stopwatch_start(&individual_timer)
    label_count, ref_count := resolve_labels(&statement_chain)
    time.stopwatch_stop(&individual_timer)
    dbgokay()
    dbg(" %fs", time.duration_seconds(time.stopwatch_duration(individual_timer)))
    set_style(ANSI.Dim)
    dbg(" (%d labels found, %d references resolved)\n", label_count, ref_count)
    set_style(ANSI.Reset)

    dbg("writing image...     ")
    time.stopwatch_reset(&individual_timer)
    time.stopwatch_start(&individual_timer)
    imgbin := make_bin(&statement_chain, predicted_len)
    time.stopwatch_stop(&individual_timer)
    defer delete(imgbin)
    writeok := os.write_entire_file(outpath, imgbin)
    if !writeok {
        die("ERR: Cannot write file \"%s\"", outpath)
    }
    dbgokay()
    dbg(" %fs", time.duration_seconds(time.stopwatch_duration(individual_timer)))
    set_style(ANSI.Dim)
    dbg(" (%d bytes written)\n", predicted_len)
    set_style(ANSI.Reset)

    // debug display statement chain
    // if display_more {       // dont clutter the terminal

    //     for i in statement_chain {

    //         if i.kind == statement_kind.Directive {
    //             set_style(ANSI.FG_Yellow)
    //         }
    //         if i.kind == statement_kind.Instruction {
    //             set_style(ANSI.FG_Red)
    //         }
    //         if i.kind != statement_kind.Label {
    //             dbg("\t\t")
    //         }

    //         dbg("%s ", i.name)
    //         set_style(ANSI.Reset)

    //         if i.kind == statement_kind.Label {
    //             dbg("\n")
    //             continue
    //         }

    //         for arg, index in i.args {
    //             if arg == (argument{}) {
    //                 dbg("_")
    //             } else if arg.kind == argument_kind.Integer {
    //                 dbg("%d", arg.value_int)
    //             } else {
    //                 dbg(arg.value_str)
    //             }

    //             if index != len(i.args)-1 {
    //                 dbg(", ")
    //             }

    //         }
    //         dbg("\n")
            
    //     }
    //     // dbg("-------------------------------------\n")
    // }

    

    time.stopwatch_stop(&overall_timer)
    dbg("assembly took %f seconds\n", time.duration_seconds(time.stopwatch_duration(overall_timer)))

}

cmd_arg :: struct {
    key : string,
    val : string,
}

replace_ext :: proc(p, new_ext: string) -> string {
    return strings.join([]string{filepath.join([]string{filepath.dir(p),  filepath.stem(p)}), new_ext}, "")
}

// init vars
inpath          : string
outpath         : string
flag_print_dbg      := false
flag_prep_only      := false
flag_ignore_ext     := false
flag_outpath_loaded := false
flag_no_color       := false