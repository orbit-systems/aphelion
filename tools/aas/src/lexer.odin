package aas

import "core:fmt"
import "core:strings"
import "core:unicode/utf8"

// lexer
// converts pure text into parse-able tokens and determines basic token types

tokenize :: proc(asm_string: string, token_chain: ^[dynamic]btoken) {
    using btoken_kind
    
    // setup
    is_new              := true
    is_comment          := false
    is_string           := false
    is_escape           := false

    // index tokens
    for char, index in asm_string {

        char_str := utf8.runes_to_string([]rune{char})

        // handle strings - shits probably so buggy
        // code now optimize --later-- never
        if char == '\"' && !is_string {
            is_string = true
            append(token_chain, btoken{Newline, char_str})
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
            append(token_chain, btoken{Newline, "\n"})
            continue
        }
        
        // handle everything else
        if is_separator(char) {     // disregard if separator
            is_new = true
            continue
        }
        if char == '\n' {           // if its a newline, reset
            is_new = true
            append(token_chain, btoken{Newline, "\n"})
            continue
        }

        if is_new {                 // if its a new word, create a new token
            append(token_chain, btoken{Unresolved, char_str})
            is_new = false
            continue
        }

        append_token_val(token_chain, char_str)
    }
    
    // determine token kinds - later revise to use regex for a more flexible system? who knows
    last_token_kind := Unresolved
    for btoken, index in token_chain^ {

        if btoken.value == "\n" {                        // mark newline
            token_chain^[index].kind = Newline
        }
        else if btoken.value[0] == '.' {                 // mark directive
            token_chain^[index].kind = Directive
        }
        else if btoken.value[top(btoken.value)] == ':' {  // mark label
            token_chain^[index].kind = Label
        }
        else if last_token_kind == Newline ||
                last_token_kind == Label {              // mark instruction
            token_chain^[index].kind = Instruction
        }
        else if is_register(btoken.value) {              // mark register
            token_chain^[index].kind = Register
        }
        else {                                          // mark literal
            token_chain^[index].kind = Literal
        }


        last_token_kind = token_chain^[index].kind
    }

}

append_token_val :: proc(token_chain: ^[dynamic]btoken, char_str: string) {
    // theres probably a better way to do this but i couldn't give less of a fuck right now
    token_chain^[top(token_chain)].value = strings.concatenate({token_chain^[top(token_chain)].value, char_str})
}



top_str :: proc(s: string) -> int {
    return len(s)-1
}

top_dyn :: proc(a: ^[dynamic]$T) -> int {
    return len(a^)-1
}

top :: proc{top_str, top_dyn}

btoken :: struct {
    kind    : btoken_kind,
    value   : string,
}

btoken_kind :: enum {
    Newline,
    Directive,
    Label,
    Instruction,
    Register,
    Literal,
    Unresolved,
}