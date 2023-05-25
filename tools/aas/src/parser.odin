package aas

import "core:fmt"
import "core:strings"
import "core:unicode/utf8"
import "core:strconv"

// parser
// converts basic tokens into statement chain, check for errors
// * ALIAS HANDLING WILL GO IN THE PREPROCESSOR

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
            if !is_directive(new_value) {
                die("\nERR [line %d]: invalid directive \"%s\"", line, tok.value)
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
            if !is_instruction(tok.value) {
                die("\nERR [line %d]: invalid instruction \"%s\"", line, tok.value)
            }

            new := statement{
                kind = statement_kind.Instruction,
                name = tok.value,
                line = line,
            }

            append(stmt_chain, new)

        case btoken_kind.Register:

            // simple name check
            if !is_register(tok.value) {    // shouldn't trigger since invalid registers are cast to literals by the lexer but whatever
                die("\nERR [line %d]: invalid register \"%s\"", line, tok.value) 
            }

            new := argument{
                argument_kind.Register,
                registers[tok.value],
                tok.value,
            }

            append(&(stmt_chain^[top(stmt_chain)].args), new)

        case btoken_kind.Literal:
            
            // recognize string literal
            if tok.value[0] == '\"' {
                if tok.value[top(tok.value)] != '\"' {
                    die("\nERR [line %d]: string not closed (%s)", line, tok.value)
                }

                new := argument{
                    kind = argument_kind.String,
                    value_str = unescape(tok.value[1:top(tok.value)]),
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
            die("\nERR [line %d]: unresolved token \"%s\"", line, tok.value)
        }
    }



}

unescape :: proc(x: string) -> string {
    return ""
}

reconstruct_line :: proc(x: statement) -> (res: string) {
    res = strings.concatenate({res, x.name})
    if x.kind == statement_kind.Label {
        return
    }
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
    Ignore,      // useful for doing argument checking - might merge this with Unresolved
    Register,
    Integer,    // any single literal data point that isn't a string, eg raw hex, 
    Symbol,
    String,
}