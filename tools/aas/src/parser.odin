package aas

import "core:fmt"
import "core:strings"
import "core:unicode/utf8"
import "core:strconv"

// parser
// converts basic tokens into statement chain, check for errors

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
            //dbg("\n[%v]\n", new)

            for arg, index in stmt_chain^[top(stmt_chain)].args {
                if arg == (argument{}) {
                    stmt_chain^[top(stmt_chain)].args[index] = new
                    break
                }
            }

        case btoken_kind.Literal:
            
            // recognize string literal
            if tok.value[0] == '\"' {
                if tok.value[top(tok.value)] != '\"' {
                    die("\nERR [line %d]: string not closed (%s)", line, tok.value)
                }
                new := argument{
                    kind = argument_kind.String,
                    value_str = tok.value[1:top(tok.value)],
                }

                for arg, index in stmt_chain^[top(stmt_chain)].args {
                    if arg == (argument{}) {
                        stmt_chain^[top(stmt_chain)].args[index] = new
                        break
                    }
                }
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
                for arg, index in stmt_chain^[top(stmt_chain)].args {
                    if arg == (argument{}) {
                        stmt_chain^[top(stmt_chain)].args[index] = new
                        break
                    }
                }
                continue
            }

            // fallback - symbol literal
            new := argument{
                kind = argument_kind.Symbol,
                value_str = tok.value,
            }
            for arg, index in stmt_chain^[top(stmt_chain)].args {
                if arg == (argument{}) {
                    stmt_chain^[top(stmt_chain)].args[index] = new
                    break
                }
            }

        case btoken_kind.Unresolved:
            die("\nERR [line %d]: unresolved token \"%s\"", line, tok.value)
        }
    }

}

check_statement_chain :: proc() {

}


statement :: struct {
    kind        : statement_kind,   // what kind of statement it is
    name        : string,           // name without formatting - eg "byte" or "string" if directive
    opcode      : int,              // opcode, if applicable
    func        : int,              // func, if applicable
    args        : [3]argument,      // arguments
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
    Register,
    Integer,    // any single literal data point that isn't a string, eg raw hex, 
    Symbol,
    String,
}

top_statement :: proc(a: ^[dynamic]statement) -> int {
    return len(a^)-1
}

hex_decode :: proc(str: string) -> (result: int, ok := true) {
    for char in str {
        result = result << 4
        result += hex_digits[char]
        if !(char in hex_digits) {
            ok = false
            return
        }
    }
    return
}

oct_decode :: proc(str: string) -> (result: int, ok := true) {
    for char in str {
        result = result << 3
        result += oct_digits[char]
        if !(char in oct_digits) {
            ok = false
            return
        }
    }
    return
}

bin_decode :: proc(str: string) -> (result: int, ok := true) {
    for char in str {
        result = result << 1
        result += bin_digits[char]
        if !(char in bin_digits) {
            ok = false
            return
        }
    }
    return
}

hex_digits := map[rune]int {
    '0' = 0,
    '1' = 1,
    '2' = 2,
    '3' = 3,
    '4' = 4,
    '5' = 5,
    '6' = 6,
    '7' = 7,
    '8' = 8,
    '9' = 9,

    'a' = 10,
    'b' = 11,
    'c' = 12,
    'd' = 13,
    'e' = 14,
    'f' = 15,

    'A' = 10,
    'B' = 11,
    'C' = 12,
    'D' = 13,
    'E' = 14,
    'F' = 15,
}

oct_digits := map[rune]int {
    '0' = 0,
    '1' = 1,
    '2' = 2,
    '3' = 3,
    '4' = 4,
    '5' = 5,
    '6' = 6,
    '7' = 7,
}

bin_digits := map[rune]int {
    '0' = 0,
    '1' = 1,
}