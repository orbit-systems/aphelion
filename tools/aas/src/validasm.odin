package aas

import "core:strings"

// this file contains everything that defines valid Aphelion assembly.

is_separator :: proc(c: rune) -> bool {
    for r in separators {
        if r == c do return true
    }
    return false
}

is_register :: proc(s: string) -> bool {
    // for r in registers {
    //     if r == strings.to_lower(s) do return true
    // }
    // return false
    return s in registers
}

is_directive :: proc(s: string) -> bool {
    for r in directives {
        if r == strings.to_lower(s) do return true
    }
    return false
}

is_instruction :: proc(s: string) -> bool {
    for r in instructions {
        if r == strings.to_lower(s) do return true
    }
    return false
}

separators :: []rune{' ', '\t', '\r', '\v', '\f', ','}

registers := map[string]int{
    "rz" = 0, 
    "ra" = 1, 
    "rb" = 2, 
    "rc" = 3, 
    "rd" = 4, 
    "re" = 5, 
    "rf" = 6, 
    "rg" = 7, 
    "rh" = 8, 
    "ri" = 9, 
    "rj" = 10, 
    "rk" = 11,    
    "pc" = 12, 
    "sp" = 13, 
    "fp" = 14, 
    "st" = 15}

instructions :: []string{
    "int",
    "inv",
    "lli",
    "llis",
    "lui",
    "luis",
    "lti",
    "ltis",
    "ltui",
    "ltuis",
    "lw",
    "lbs",
    "lb",
    "st",
    "stb",
    "swp",
    "add",
    "adc",
    "sub",
    "sbb",
    "mul",
    "div",
    "mod",
    "and",
    "or",
    "nor",
    "xor",
    "shl",
    "asr",
    "lsr",
    "push",
    "pushi",
    "pushz",
    "pushc",
    "pop",
    "enter",
    "leave",
    "reloc",
    "jal",
    "jalr",
    "ret",
    "retr",
    "beq",
    "bez",
    "blt",
    "ble",
    "bltu",
    "bleu",
    "bpe",
    "bne",
    "bnz",
    "bge",
    "bgt",
    "bgeu",
    "bgtu",
    "bpd"}

directives :: []string{
    "raw",
    "rawle",
    "rawbe",
    "u8",
    "u16",
    "u32",
    "u64",
    "u128",
    "u256",
    "i8",
    "i16",
    "i32",
    "i64",
    "i128",
    "i256",
    "byte",
    "string",
    "addr",
    "loc",
    "skip",
    "align"}