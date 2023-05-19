package aas

import "core:fmt"
import "core:strings"
import "core:unicode/utf8"

// lexer
// converts pure text into parse-able tokens and determines basic token types

tokenize :: proc(asm_string: string, token_chain: ^[dynamic]aphel_token) {
    using aphel_token_kind
    
    // setup
    is_new              := true
    is_comment          := false
    is_string           := false
    is_escape           := false

    // index tokens
    for char, index in asm_string {

        char_str := utf8.runes_to_string([]rune{char})

        // handle strings - shits probably so buggy
        // code now optimize later
        if char == '\"' && !is_string {
            is_string = true
            append(token_chain, aphel_token{Newline, char_str})
            continue
        }
        if is_string {
            if char == '\"' && !is_escape {
                is_string = false
                is_escape = false
                is_new = true
            } else if char == '\"' && is_escape {
                is_escape = false
            } else if char == '\\' && !is_escape {
                is_escape = true
            } else if is_escape {
                is_escape = false
            }
            append_token_val(token_chain, char_str)
            continue
        }
        
        
        // handle comments
        if char == '#' {            // detect comment start
            is_comment = true
        }
        if is_comment && char != '\n' { // disregard if inside comment
            continue
        }
        if is_comment && char == '\n' {  // reset if newline
            is_new = true
            is_comment = false
            append(token_chain, aphel_token{Newline, "\n"})
            continue
        }
        

        // handle everything else
        if is_separator(char) {     // disregard if separator
            is_new = true
            continue
        }
        if char == '\n' {           // if its a newline, reset
            is_new = true
            append(token_chain, aphel_token{Newline, "\n"})
            continue
        }

        if is_new {                 // if its a new word, create a new token
            append(token_chain, aphel_token{Unresolved, char_str})
            is_new = false
            continue
        }

        append_token_val(token_chain, char_str)
    }
    
    // determine token kinds
    last_token_kind := Unresolved
    for token, index in token_chain^ {

        if token.value == "\n" {                        // mark newline
            token_chain^[index].kind = Newline
        }
        else if token.value[0] == '.' {                 // mark directive
            token_chain^[index].kind = Directive
        }
        else if token.value[top(token.value)] == ':' {  // mark label
            token_chain^[index].kind = Label
        }
        else if last_token_kind == Newline ||
                last_token_kind == Label {              // mark instruction
            token_chain^[index].kind = Instruction
        }
        else if is_register(token.value) {              // mark register
            token_chain^[index].kind = Register
        }
        else {                                          // mark literal
            token_chain^[index].kind = Literal
        }


        last_token_kind = token_chain^[index].kind
    }

}

append_token_val :: proc(token_chain: ^[dynamic]aphel_token, char_str: string) {
    // theres probably a better way to do this but i couldn't give less of a fuck right now
    token_chain^[top(token_chain)].value = strings.concatenate({token_chain^[top(token_chain)].value, char_str})
}

is_separator :: proc(c: rune) -> bool {
    for r in separators {
        if r == c do return true
    }
    return false
}

is_register :: proc(s: string) -> bool {
    for r in registers {
        if r == strings.to_lower(s) do return true
    }
    return false
}

top_str :: proc(s: string) -> int {
    return len(s)-1
}

top_dyn_aph_token :: proc(a: ^[dynamic]aphel_token) -> int {
    return len(a^)-1
}

top :: proc{top_str, top_dyn_aph_token}

separators := []rune{' ', '\t', '\r', '\v', '\f', ','}

registers := []string{"rz", "ra", "rb", "rc", "rd", "re", "rf", "rg", "rh", "ri", "rj", "rk",
                      "pc", "sp", "fp", "st"}

aphel_token :: struct {
    kind    : aphel_token_kind,
    value   : string,
}

aphel_token_kind :: enum {
    Newline,
    Directive,
    Label,
    Instruction,
    Register,
    Literal,
    Unresolved,
}