package aas

import "core:fmt"
import "core:strings"
import "core:unicode/utf8"

tokenize :: proc(asm_string: string, token_chain: ^[dynamic]aphel_token) {
    using aphel_token_kind
    
    // add all tokens
    is_new := true
    for char, index in asm_string {
        
        if is_separator(char) {     // skip if separator
            is_new = true
            continue
        }
        x := utf8.runes_to_string([]rune{char})
        if is_new {                 // if its a new word, create a new token
            
            append(token_chain, aphel_token{Unresolved, x})
            if char != '\n' {       // if its a newline, create a new token and reset
                is_new = false
            }
            continue
        }
        // theres probably a better way to do this but i couldn't give less of a fuck right now
        token_chain^[top(token_chain)].value = strings.concatenate({token_chain^[top(token_chain)].value, x})
    }

    // {
    //     for i in token_chain^ {
    //         if i.value == "\n" {
    //             dbg("\n")
    //             continue
    //         }
    //         dbg("%s ", i.value)
    //     }
    // }
    
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
                last_token_kind == Label {              // mark label
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