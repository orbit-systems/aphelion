package aas

import "core:fmt"
import "core:os"
import "core:strings"
import "core:unicode/utf8"
import "core:strconv"

// parser
// converts basic tokens into statement chain, check for errors
// * ALIAS HANDLING WILL GO IN THE PREPROCESSOR
// TODO jal with label support

construct_statement_chain :: proc(stmt_chain: ^[dynamic]statement, tokens: ^[dynamic]btoken) {



    // construct chain
    line := 1
    current_addr := 0
    for tok in tokens^ {
        switch tok.kind {
        case btoken_kind.Newline:
            line += 1
        case btoken_kind.Directive:

            new_value := tok.value[1:]  //get rid of '.'
            // simple name check
            if !is_native_directive(new_value) {
                die("ERR [line %d]: invalid directive \"%s\"", line, tok.value)
            }

            new := statement{
                kind = statement_kind.Directive,
                name = new_value,
                line = line,
            }

            append(stmt_chain, new)

        case btoken_kind.Label:

            new := statement{
                kind = statement_kind.Label,
                name = tok.value[:len(tok.value)-1],
                line = line,
            }

            append(stmt_chain, new)

        case btoken_kind.Instruction:

            // simple name check
            if !is_native_instruction(tok.value) {
                die("ERR [line %d]: invalid instruction \"%s\"", line, tok.value)
            }

            new := statement{
                kind    = statement_kind.Instruction,
                opcode  = native_instruction_opcodes[strings.to_lower(tok.value)][0],
                func    = native_instruction_opcodes[strings.to_lower(tok.value)][1],
                name    = strings.to_lower(tok.value), // normalize
                line    = line,
            }

            append(stmt_chain, new)

        case btoken_kind.Register:

            // simple name check
            if !is_register(tok.value) {    // shouldn't trigger since invalid registers are cast to literals by the lexer but whatever
                die("ERR [line %d]: invalid register \"%s\"", line, tok.value) 
            }

            norm := strings.to_lower(tok.value) // normalize

            new := argument{
                argument_kind.Register,
                registers[norm],
                norm, // normalize
            }

            append(&(stmt_chain^[top(stmt_chain)].args), new)

        case btoken_kind.Literal:
            
            // recognize string literal
            if tok.value[0] == '\"' {
                if tok.value[top(tok.value)] != '\"' {
                    die("ERR [line %d]: string not closed (%s)", line, tok.value)
                }
                unsc, ok := unescape(tok.value[1:top(tok.value)])
                if ok != "" {
                    die("ERR [line %d]: escape sequence not recognized \"%s\"", line, ok)
                }

                new := argument{
                    kind = argument_kind.String,
                    value_str = unsc,
                }
                append(&(stmt_chain^[top(stmt_chain)].args), new)
                continue
            }

            // recognize integer literal
            // thank god for strconv, this section was like 100 lines long before i rembered it existed 0.0
            decoded, ok := strconv.parse_int(tok.value)
            if ok {
                new := argument{
                    kind = argument_kind.Integer,
                    value_int = decoded,
                }
                append(&(stmt_chain^[top(stmt_chain)].args), new)
                // add_arg_to_statement(&(stmt_chain^[top(stmt_chain)]), new)
                continue
            }

            // fallback - symbol literal
            new := argument{
                kind = argument_kind.Symbol,
                value_str = tok.value,
            }
            append(&(stmt_chain^[top(stmt_chain)].args), new)

        case btoken_kind.Unresolved:
            die("ERR [line %d]: unresolved token \"%s\"", line, tok.value)
        }
    }



    // these are not the arguments your are looking for (check arguments)
    for st in stmt_chain^ {

        args : [dynamic]argument_kind
        defer delete(args)

        for starg in st.args {
            append(&args, starg.kind)
        }

        switch st.kind {
        case statement_kind.Directive:
            ref_args := native_directive_args[st.name]

            if len(ref_args) != len(args) {
                die("ERR [line %d]: invalid arguments for \".%s\" - expected %v, got %v", st.line, st.name, ref_args, args)
            }
            for i := 0; i < len(ref_args); i += 1 {
                if ref_args[i] != args[i] {
                    die("ERR [line %d]: invalid arguments for \".%s\" - expected %v, got %v", st.line, st.name, ref_args, args)
                }
            }
            
        case statement_kind.Instruction:
        
            ref_args := native_instruction_args[st.name]

            if len(ref_args) != len(args) {
                die("ERR [line %d]: invalid arguments for \"%s\" - expected %v, got %v", st.line, st.name, ref_args, args)
            }
            for i := 0; i < len(ref_args); i += 1 {
                if ref_args[i] != args[i] {
                    die("ERR [line %d]: invalid arguments for \"%s\" - expected %v, got %v", st.line, st.name, ref_args, args)
                }
            }

        case statement_kind.Label:
            if len(st.args) != 0 {
                die("ERR [line %d]: something has gone very wrong, contact me immediately", st.line)
            }
        case statement_kind.Unresolved:
            die("ERR [line %d]: unresolved statement (how did this even happen) \"%s\"", st.line, st.name)
            
        }

    }


    // trace statement chain, fill in LOC and SIZE values - optimize / clean up later
    img_pointer := 0
    img_size := 0
    for st, index in stmt_chain^ {
        switch st.kind {
        case statement_kind.Unresolved:
            die("ERR [line %d]: wtf", st.line)
        case statement_kind.Instruction:
            stmt_chain^[index].loc = img_pointer
            stmt_chain^[index].size = 4
        case statement_kind.Label:
            stmt_chain^[index].loc = img_pointer
            stmt_chain^[index].size = 0
        case statement_kind.Directive:
            // spent the last 30 minutes reconsidering the directive system lmao
            switch strings.to_lower(st.name) {
            
            // embedding
            case "u8", "i8", "u8be", "i8be":
                stmt_chain^[index].loc = img_pointer
                stmt_chain^[index].size = 1
            case "u16", "i16", "u16be", "i16be":
                stmt_chain^[index].loc = img_pointer
                stmt_chain^[index].size = 2
            case "u32", "i32", "u32be", "i32be":
                stmt_chain^[index].loc = img_pointer
                stmt_chain^[index].size = 4
            case "u64", "i64", "u64be", "i64be":
                stmt_chain^[index].loc = img_pointer
                stmt_chain^[index].size = 8
            case "u128", "i128", "u128be", "i128be":
                stmt_chain^[index].loc = img_pointer
                stmt_chain^[index].size = 16
            case "byte":
                stmt_chain^[index].loc = img_pointer
                stmt_chain^[index].size = st.args[1].value_int
            case "string":
                stmt_chain^[index].loc = img_pointer
                stmt_chain^[index].size = len(st.args[0].value_str) //byte length of string should be the correct size because it has been unescaped
            case "val":
                stmt_chain^[index].loc = img_pointer
                stmt_chain^[index].size = 8     // 64 bits
            case "bin":
                stmt_chain^[index].loc = img_pointer
                binfile, ok := os.read_entire_file(st.args[0].value_str)
                if !ok {
                    die("ERR [line %d]: cannot find file at \"%s\"", st.line, st.args[0].value_str)
                }
                stmt_chain^[index].size = len(binfile)
                delete(binfile)

            // sectioning
            case "loc":
                img_pointer = st.args[0].value_int
                stmt_chain^[index].loc = img_pointer
                stmt_chain^[index].size = 0
            case "skip":
                img_pointer += st.args[0].value_int
                stmt_chain^[index].loc = img_pointer
                stmt_chain^[index].size = 0
            case "align":
                a := st.args[0].value_int
                img_pointer = img_pointer if img_pointer % a == 0 else ( img_pointer / a + 1) * a // round up to next multiple of (a)
                stmt_chain^[index].loc = img_pointer
                stmt_chain^[index].size = 0
            }
        }

        img_pointer += stmt_chain^[index].size
        img_size = max(img_size, img_pointer)
    }
    
    // build symbol table -- old 2mil test file breaks here!! i'll have to write a program to generate a valid one
    symbol_table := make(map[string]int)
    defer delete(symbol_table)
    for st in stmt_chain^ {
        if st.kind != statement_kind.Label {
            continue
        }
        if st.name in symbol_table {
            die("ERR [line %d]: duplicate label \"%s\"", st.line, st.name)
        }
        symbol_table[st.name] = st.loc
    }

    // symbol replacement
    for st in stmt_chain^ {

        // val directive
        if st.kind == statement_kind.Directive && strings.to_lower(st.name) == "val" {
            addr, ok := symbol_table[st.args[0].value_str]
            if !ok {
                die("ERR [line %d]: symbol not declared \"%s\"", st.line, st.args[0].value_str)
            }
            st.args[0].value_int = addr
            continue
        }

        // if branch instruction
        if st.kind == statement_kind.Instruction && st.opcode == 0x63 {
            addr, ok := symbol_table[st.args[0].value_str]
            if !ok {
                die("ERR [line %d]: symbol not declared \"%s\"", st.line, st.args[0].value_str)
            }
            // the branch instruction can only jump in increments of 4 bytes, so the label and the instruction must be aligned
            diff := addr - st.loc
            if (diff % 4 != 0) {
                die("ERR [line %d]: label \"%s\" is unaligned with branch, cannot resolve \"%s\"", st.line, st.args[0].value_str, st.name)
            }
            st.args[0].value_int = diff / 4
        }
    }
}

unescape :: proc(x: string) -> (res: string, err := "") {
    prev_slash := false
    for c, index in x {
        if !prev_slash && c != '\\' {
            res = strings.concatenate({res, utf8.runes_to_string({c})})
        }
        if !prev_slash && c == '\\' {
            prev_slash = true
            continue
        }
        if prev_slash {
            esc, ok := escape_seqs[utf8.rune_string_at_pos(x, index)]
            if !ok {
                res = ""
                err = strings.concatenate({"\\", utf8.rune_string_at_pos(x, index)})
                return
            }
            res = strings.concatenate({res, esc})
            prev_slash = false
        }
    }
    return
}

// only really useful for error messages and debug - might delete if nothing relies on it
reconstruct_line :: proc(x: statement) -> (res: string) {
    if x.kind == statement_kind.Label {
        res = strings.concatenate({x.name, ":"})
        return
    }
    if x.kind == statement_kind.Directive {
        res = strings.concatenate({".", x.name})

    }
    if x.kind == statement_kind.Instruction {
        res = x.name
    }
    res = strings.concatenate({res," "})
    for arg, index in x.args {
        if arg == (argument{}) {
            res = strings.concatenate({res, "_"})
        } else if arg.kind == argument_kind.Integer {
            res = strings.concatenate({res, fmt.aprintf("%d", arg.value_int)})
        } else {
            res = strings.concatenate({res, arg.value_str})
        }

        if index != len(x.args)-1 {
            res = strings.concatenate({res, ", "})
        }
    }
    
    return
}

statement :: struct {
    kind        : statement_kind,   // what kind of statement it is
    name        : string,           // name without formatting - eg "byte" or "string" if directive
    opcode      : int,              // opcode, if applicable
    func        : int,              // func, if applicable
    args        : [dynamic]argument,// arguments
    line        : int,              // line number for error display

    loc         : int,              // location in image
    size        : int,              // size of statement in bytes
}

argument :: struct {
    kind        : argument_kind,    // what kind of argument it is
    value_int   : int,              // int value, if applicable
    value_str   : string,           // string value, if applicable
}

statement_kind :: enum {
    Unresolved = 0,
    Instruction,
    Directive,
    Label,
}

argument_kind :: enum {
    Unresolved = 0,
    Ignore,      // was useful for doing argument checking - might delete now that it's not really that useful
    Register,
    Integer,    // any single literal data point that isn't a string, eg raw hex, 
    Symbol,
    String,
}