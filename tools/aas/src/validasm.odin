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
    return strings.to_lower(s) in registers
}

is_native_directive :: proc(s: string) -> bool {
    // for r in directives {
    //     if r == strings.to_lower(s) do return true
    // }
    // return false
    return strings.to_lower(s) in native_directive_args
}

is_native_instruction :: proc(s: string) -> bool {
    // for r in instructions {
    //     if r == strings.to_lower(s) do return true
    // }
    // return false
    return strings.to_lower(s) in native_instruction_args
}

separators :: []rune{' ', '\t', '\r', '\v', '\f', ','}

escape_seqs := map[string]string{
    "0"   = "\x00",
    "n"   = "\n",
    "\\"  = "\\",
    "\""  = "\"",
    "\'"  = "\'",
}

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
    "st" = 15,
}

ak :: argument_kind

// these are the only instructions that should reach the lexer and parser. these have definite arguments.
native_instruction_args := map[string][]argument_kind{
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
    "bra"   = []ak{ak.Symbol},
    "beq"   = []ak{ak.Symbol},
    "bez"   = []ak{ak.Symbol},
    "blt"   = []ak{ak.Symbol},
    "ble"   = []ak{ak.Symbol},
    "bltu"  = []ak{ak.Symbol},
    "bleu"  = []ak{ak.Symbol},
    "bpe"   = []ak{ak.Symbol},
    "bne"   = []ak{ak.Symbol},
    "bnz"   = []ak{ak.Symbol},
    "bge"   = []ak{ak.Symbol},
    "bgt"   = []ak{ak.Symbol},
    "bgeu"  = []ak{ak.Symbol},
    "bgtu"  = []ak{ak.Symbol},
    "bpd"   = []ak{ak.Symbol},
}

native_instruction_opcodes := map[string][]int{
                //  op, func
    "nop"   = []int{0x0A, 0}
    "int"   = []int{0x10, 0}
    "inv"   = []int{0x11, 0}
    "usr"   = []int{0x12, 0}

    "lli"   = []int{0x20, 0}
    "llis"  = []int{0x20, 1}
    "lui"   = []int{0x20, 2}
    "luis"  = []int{0x20, 3}
    "lti"   = []int{0x20, 4}
    "ltis"  = []int{0x20, 5}
    "ltui"  = []int{0x20, 6}
    "ltuis" = []int{0x20, 7}
    "lw"    = []int{0x21, 0}
    "lbs"   = []int{0x22, 0}
    "lb"    = []int{0x23, 0}
    "st"    = []int{0x24, 0}
    "stb"   = []int{0x25, 0}
    "swp"   = []int{0x26, 0}

    "addr"  = []int{0x30, 0}
    "addi"  = []int{0x31, 0}
    "adcr"  = []int{0x32, 0}
    "adci"  = []int{0x33, 0}
    "subr"  = []int{0x34, 0}
    "subi"  = []int{0x35, 0}
    "sbbr"  = []int{0x36, 0}
    "sbbi"  = []int{0x37, 0}
    "mulr"  = []int{0x38, 0}
    "muli"  = []int{0x39, 0}
    "divr"  = []int{0x3a, 0}
    "divi"  = []int{0x3b, 0}

    "andr"  = []int{0x40, 0}
    "andi"  = []int{0x41, 0}
    "orr"   = []int{0x42, 0}
    "ori"   = []int{0x43, 0}
    "norr"  = []int{0x44, 0}
    "nori"  = []int{0x45, 0}
    "xorr"  = []int{0x46, 0}
    "xori"  = []int{0x47, 0}
    "shlr"  = []int{0x48, 0}
    "shli"  = []int{0x49, 0}
    "asrr"  = []int{0x4a, 0}
    "asri"  = []int{0x4b, 0}
    "lsrr"  = []int{0x4c, 0}
    "lsri"  = []int{0x4d, 0}

    "push"  = []int{0x50, 0}
    "pushi" = []int{0x50, 1}
    "pushz" = []int{0x50, 2}
    "pushc" = []int{0x50, 3}
    "pop"   = []int{0x50, 4}
    "enter" = []int{0x50, 5}
    "leave" = []int{0x50, 6}
    "reloc" = []int{0x50, 7}

    "jal"   = []int{0x60, 0}
    "jalr"  = []int{0x61, 0}
    "ret"   = []int{0x62, 0}
    "retr"  = []int{0x62, 0}
    "beq"   = []int{0x63, 0x1}
    "bez"   = []int{0x63, 0x2}
    "blt"   = []int{0x63, 0x3}
    "ble"   = []int{0x63, 0x4}
    "bltu"  = []int{0x63, 0x5}
    "bleu"  = []int{0x63, 0x6}
    "bpe"   = []int{0x63, 0x7}
    "bne"   = []int{0x63, 0x9}
    "bnz"   = []int{0x63, 0xa}
    "bge"   = []int{0x63, 0xb}
    "bgt"   = []int{0x63, 0xc}
    "bgeu"  = []int{0x63, 0xd}
    "bgtu"  = []int{0x63, 0xe}
    "bpd"   = []int{0x63, 0xf}
}

native_directive_args := map[string][]argument_kind{
    "u8"     = []ak{ak.Integer},
    "u16"    = []ak{ak.Integer},
    "u32"    = []ak{ak.Integer},
    "u64"    = []ak{ak.Integer},
    "u128"   = []ak{ak.Integer},
    "u256"   = []ak{ak.Integer},
    "i8"     = []ak{ak.Integer},
    "i16"    = []ak{ak.Integer},
    "i32"    = []ak{ak.Integer},
    "i64"    = []ak{ak.Integer},
    "i128"   = []ak{ak.Integer},
    "i256"   = []ak{ak.Integer},
    "u8be"   = []ak{ak.Integer},
    "u16be"  = []ak{ak.Integer},
    "u32be"  = []ak{ak.Integer},
    "u64be"  = []ak{ak.Integer},
    "u128be" = []ak{ak.Integer},
    "i8be"   = []ak{ak.Integer},
    "i16be"  = []ak{ak.Integer},
    "i32be"  = []ak{ak.Integer},
    "i64be"  = []ak{ak.Integer},
    "i128be" = []ak{ak.Integer},
    "byte"   = []ak{ak.Integer, ak.Integer},
    "string" = []ak{ak.String},
    "val"    = []ak{ak.Symbol},
    "bin"    = []ak{ak.String},

    "loc"    = []ak{ak.Integer},
    "skip"   = []ak{ak.Integer},
    "align"  = []ak{ak.Integer},
}

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