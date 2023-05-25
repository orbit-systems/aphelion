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
    // for r in instructions {
    //     if r == strings.to_lower(s) do return true
    // }
    // return false
    return s in instruction_args
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

ak :: argument_kind

instruction_args := map[string][]argument_kind{
    "nop"   = []ak{},
    "int"   = []ak{ak.Integer},
    "inv"   = []ak{ak.Integer, ak.Register, ak.Register},
    "usr"   = []ak{},

    "lli"   = []ak{ak.Register, ak.Integer},
    "llis"  = []ak{ak.Register, ak.Integer},
    "lui"   = []ak{ak.Register, ak.Integer},
    "luis"  = []ak{ak.Register, ak.Integer},
    "lti"   = []ak{ak.Register, ak.Integer},
    "ltis"  = []ak{ak.Register, ak.Integer},
    "ltui"  = []ak{ak.Register, ak.Integer},
    "ltuis" = []ak{ak.Register, ak.Integer},
    "lw"    = []ak{ak.Register, ak.Register, ak.Integer},
    "lbs"   = []ak{ak.Register, ak.Register, ak.Integer},
    "lb"    = []ak{ak.Register, ak.Register, ak.Integer},
    "st"    = []ak{ak.Register, ak.Register, ak.Integer},
    "stb"   = []ak{ak.Register, ak.Register, ak.Integer},
    "swp"   = []ak{ak.Register, ak.Register},

    "addr"  = []ak{ak.Register, ak.Register, ak.Register},
    "addi"  = []ak{ak.Register, ak.Register, ak.Integer},
    "adcr"  = []ak{ak.Register, ak.Register, ak.Register},
    "adci"  = []ak{ak.Register, ak.Register, ak.Integer},
    "subr"  = []ak{ak.Register, ak.Register, ak.Register},
    "subi"  = []ak{ak.Register, ak.Register, ak.Integer},
    "sbbr"  = []ak{ak.Register, ak.Register, ak.Register},
    "sbbi"  = []ak{ak.Register, ak.Register, ak.Integer},
    "mulr"  = []ak{ak.Register, ak.Register, ak.Register},
    "muli"  = []ak{ak.Register, ak.Register, ak.Integer},
    "divr"  = []ak{ak.Register, ak.Register, ak.Register},
    "divi"  = []ak{ak.Register, ak.Register, ak.Integer},

    "andr"  = []ak{ak.Register, ak.Register, ak.Register},
    "andi"  = []ak{ak.Register, ak.Register, ak.Integer},
    "orr"   = []ak{ak.Register, ak.Register, ak.Register},
    "ori"   = []ak{ak.Register, ak.Register, ak.Integer},
    "norr"  = []ak{ak.Register, ak.Register, ak.Register},
    "nori"  = []ak{ak.Register, ak.Register, ak.Integer},
    "xorr"  = []ak{ak.Register, ak.Register, ak.Register},
    "xori"  = []ak{ak.Register, ak.Register, ak.Integer},
    "shlr"  = []ak{ak.Register, ak.Register, ak.Register},
    "shli"  = []ak{ak.Register, ak.Register, ak.Integer},
    "asrr"  = []ak{ak.Register, ak.Register, ak.Register},
    "asri"  = []ak{ak.Register, ak.Register, ak.Integer},
    "lsrr"  = []ak{ak.Register, ak.Register, ak.Register},
    "lsri"  = []ak{ak.Register, ak.Register, ak.Integer},
    "push"  = []ak{ak.Register},
    "pushi" = []ak{ak.Integer},
    "pushz" = []ak{ak.Integer},
    "pushc" = []ak{ak.Integer},
    "pop"   = []ak{ak.Register},
    "enter" = []ak{},
    "leave" = []ak{},
    "reloc" = []ak{ak.Register, ak.Integer},
    "jal"   = []ak{ak.Register, ak.Integer},
    "jalr"  = []ak{ak.Register, ak.Integer, ak.Register},
    "ret"   = []ak{},
    "retr"  = []ak{ak.Register},
    "beq"   = []ak{ak.Integer},
    "bez"   = []ak{ak.Integer},
    "blt"   = []ak{ak.Integer},
    "ble"   = []ak{ak.Integer},
    "bltu"  = []ak{ak.Integer},
    "bleu"  = []ak{ak.Integer},
    "bpe"   = []ak{ak.Integer},
    "bne"   = []ak{ak.Integer},
    "bnz"   = []ak{ak.Integer},
    "bge"   = []ak{ak.Integer},
    "bgt"   = []ak{ak.Integer},
    "bgeu"  = []ak{ak.Integer},
    "bgtu"  = []ak{ak.Integer},
    "bpd"   = []ak{ak.Integer},
}

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

// hex_digits := map[rune]int {
//     '0' = 0,
//     '1' = 1,
//     '2' = 2,
//     '3' = 3,
//     '4' = 4,
//     '5' = 5,
//     '6' = 6,
//     '7' = 7,
//     '8' = 8,
//     '9' = 9,

//     'a' = 10,
//     'b' = 11,
//     'c' = 12,
//     'd' = 13,
//     'e' = 14,
//     'f' = 15,

//     'A' = 10,
//     'B' = 11,
//     'C' = 12,
//     'D' = 13,
//     'E' = 14,
//     'F' = 15,
// }

// oct_digits := map[rune]int {
//     '0' = 0,
//     '1' = 1,
//     '2' = 2,
//     '3' = 3,
//     '4' = 4,
//     '5' = 5,
//     '6' = 6,
//     '7' = 7,
// }

// bin_digits := map[rune]int {
//     '0' = 0,
//     '1' = 1,
// }