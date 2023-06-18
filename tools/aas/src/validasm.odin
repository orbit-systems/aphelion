package aas

import "core:strings"

// this file contains everything that defines valid Aphelion assembly.
// todo put everything about the instructions in a single map, with an info struct or smth
// * if i make this comprehensive enough, maybe i can use this to assemble different instruction sets

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

separators      :: []rune{' ', '\t', '\r', '\v', '\f', ','}
separators_str  :: []string{" ", "\t", "\r", "\v", "\f", ","}

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

instruction_aliases := map[string][]string{
    "add"   = []string{"addi", "addr"},
    "adc"   = []string{"adci", "adcr"},
    "sub"   = []string{"subi", "subr"},
    "sbb"   = []string{"sbbi", "sbbr"},
    "mul"   = []string{"muli", "mulr"},
    "div"   = []string{"divi", "divr"},

    "and"   = []string{"andi", "andr"},
    "or"    = []string{"ori", "orr"},
    "nor"   = []string{"nori", "norr"},
    "xor"   = []string{"xori", "xorr"},
    "shl"   = []string{"shli", "shlr"},
    "asr"   = []string{"asri", "asrr"},
    "lsr"   = []string{"lsri", "lsrr"},
}

ak :: argument_kind
native_instruction_args := map[string][]argument_kind{
    "nop"   = []ak{},
    "int"   = []ak{ak.Integer},
    "inv"   = []ak{},
    "usr"   = []ak{},

    "lli"   = []ak{ak.Register, ak.Integer},
    "llis"  = []ak{ak.Register, ak.Integer},
    "lui"   = []ak{ak.Register, ak.Integer},
    "luis"  = []ak{ak.Register, ak.Integer},
    "lti"   = []ak{ak.Register, ak.Integer},
    "ltis"  = []ak{ak.Register, ak.Integer},
    "ltui"  = []ak{ak.Register, ak.Integer},
    "ltuis" = []ak{ak.Register, ak.Integer},
    "ld"    = []ak{ak.Register, ak.Register, ak.Integer},
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

    "ljal"  = []ak{ak.Register, ak.Integer},
    "ljalr" = []ak{ak.Register, ak.Integer, ak.Register},
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
    "jal"   = []ak{ak.Symbol},
    "jalr"  = []ak{ak.Symbol, ak.Register},
}

native_directive_args := map[string][]argument_kind{
    "u8"     = []ak{ak.Integer},
    "u16"    = []ak{ak.Integer},
    "u32"    = []ak{ak.Integer},
    "u64"    = []ak{ak.Integer},
    "i8"     = []ak{ak.Integer},
    "i16"    = []ak{ak.Integer},
    "i32"    = []ak{ak.Integer},
    "i64"    = []ak{ak.Integer},
    "u8be"   = []ak{ak.Integer},
    "u16be"  = []ak{ak.Integer},
    "u32be"  = []ak{ak.Integer},
    "u64be"  = []ak{ak.Integer},
    "i8be"   = []ak{ak.Integer},
    "i16be"  = []ak{ak.Integer},
    "i32be"  = []ak{ak.Integer},
    "i64be"  = []ak{ak.Integer},
    "byte"   = []ak{ak.Integer, ak.Integer},
    "string" = []ak{ak.String},
    "val"    = []ak{ak.Symbol},
    "bin"    = []ak{ak.String},

    "loc"    = []ak{ak.Integer},
    "skip"   = []ak{ak.Integer},
    "align"  = []ak{ak.Integer},
}

native_instruction_opcodes := map[string][]int{
                //  op, func
    "nop"   = []int{0x0A, 0},
    "int"   = []int{0x10, 0},
    "inv"   = []int{0x11, 0},
    "usr"   = []int{0x12, 0},

    "lli"   = []int{0x20, 0},
    "llis"  = []int{0x20, 1},
    "lui"   = []int{0x20, 2},
    "luis"  = []int{0x20, 3},
    "lti"   = []int{0x20, 4},
    "ltis"  = []int{0x20, 5},
    "ltui"  = []int{0x20, 6},
    "ltuis" = []int{0x20, 7},
    "ld"    = []int{0x21, 0},
    "lbs"   = []int{0x22, 0},
    "lb"    = []int{0x23, 0},
    "st"    = []int{0x24, 0},
    "stb"   = []int{0x25, 0},
    "swp"   = []int{0x26, 0},

    "addr"  = []int{0x30, 0},
    "addi"  = []int{0x31, 0},
    "adcr"  = []int{0x32, 0},
    "adci"  = []int{0x33, 0},
    "subr"  = []int{0x34, 0},
    "subi"  = []int{0x35, 0},
    "sbbr"  = []int{0x36, 0},
    "sbbi"  = []int{0x37, 0},
    "mulr"  = []int{0x38, 0},
    "muli"  = []int{0x39, 0},
    "divr"  = []int{0x3a, 0},
    "divi"  = []int{0x3b, 0},

    "andr"  = []int{0x40, 0},
    "andi"  = []int{0x41, 0},
    "orr"   = []int{0x42, 0},
    "ori"   = []int{0x43, 0},
    "norr"  = []int{0x44, 0},
    "nori"  = []int{0x45, 0},
    "xorr"  = []int{0x46, 0},
    "xori"  = []int{0x47, 0},
    "shlr"  = []int{0x48, 0},
    "shli"  = []int{0x49, 0},
    "asrr"  = []int{0x4a, 0},
    "asri"  = []int{0x4b, 0},
    "lsrr"  = []int{0x4c, 0},
    "lsri"  = []int{0x4d, 0},

    "push"  = []int{0x50, 0},
    "pushi" = []int{0x50, 1},
    "pushz" = []int{0x50, 2},
    "pushc" = []int{0x50, 3},
    "pop"   = []int{0x50, 4},
    "enter" = []int{0x50, 5},
    "leave" = []int{0x50, 6},
    "reloc" = []int{0x50, 7},

    "ljal"  = []int{0x60, 0},
    "ljalr" = []int{0x61, 0},
    "ret"   = []int{0x62, 0},
    "retr"  = []int{0x62, 1},
    "jal"   = []int{0x64, 0},
    "jalr"  = []int{0x65, 0},

    "bra"   = []int{0x63, 0x0},
    "beq"   = []int{0x63, 0x1},
    "bez"   = []int{0x63, 0x2},
    "blt"   = []int{0x63, 0x3},
    "ble"   = []int{0x63, 0x4},
    "bltu"  = []int{0x63, 0x5},
    "bleu"  = []int{0x63, 0x6},
    "bpe"   = []int{0x63, 0x7},
    "bne"   = []int{0x63, 0x9},
    "bnz"   = []int{0x63, 0xa},
    "bge"   = []int{0x63, 0xb},
    "bgt"   = []int{0x63, 0xc},
    "bgeu"  = []int{0x63, 0xd},
    "bgtu"  = []int{0x63, 0xe},
    "bpd"   = []int{0x63, 0xf},
}

instruction_fmt :: enum {
    R, M, F, J, B,
}

instruction_fmt_fields :: enum {
    RDE, RS1, RS2, IMM,
}

// maps the instruction arguments to embedded fields
iff :: instruction_fmt_fields
native_instruction_args_to_fields := map[string][]iff{
    "nop"   = []iff{},
    "int"   = []iff{iff.IMM},
    "inv"   = []iff{},
    "usr"   = []iff{},

    "lli"   = []iff{iff.RDE, iff.IMM},
    "llis"  = []iff{iff.RDE, iff.IMM},
    "lui"   = []iff{iff.RDE, iff.IMM},
    "luis"  = []iff{iff.RDE, iff.IMM},
    "lti"   = []iff{iff.RDE, iff.IMM},
    "ltis"  = []iff{iff.RDE, iff.IMM},
    "ltui"  = []iff{iff.RDE, iff.IMM},
    "ltuis" = []iff{iff.RDE, iff.IMM},
    "ld"    = []iff{iff.RDE, iff.RS1, iff.IMM},
    "lbs"   = []iff{iff.RDE, iff.RS1, iff.IMM},
    "lb"    = []iff{iff.RDE, iff.RS1, iff.IMM},
    "st"    = []iff{iff.RS1, iff.RS2, iff.IMM},
    "stb"   = []iff{iff.RS1, iff.RS2, iff.IMM},
    "swp"   = []iff{iff.RS1, iff.RS2},

    "addr"  = []iff{iff.RDE, iff.RS1, iff.RS2},
    "addi"  = []iff{iff.RDE, iff.RS1, iff.IMM},
    "adcr"  = []iff{iff.RDE, iff.RS1, iff.RS2},
    "adci"  = []iff{iff.RDE, iff.RS1, iff.IMM},
    "subr"  = []iff{iff.RDE, iff.RS1, iff.RS2},
    "subi"  = []iff{iff.RDE, iff.RS1, iff.IMM},
    "sbbr"  = []iff{iff.RDE, iff.RS1, iff.RS2},
    "sbbi"  = []iff{iff.RDE, iff.RS1, iff.IMM},
    "mulr"  = []iff{iff.RDE, iff.RS1, iff.RS2},
    "muli"  = []iff{iff.RDE, iff.RS1, iff.IMM},
    "divr"  = []iff{iff.RDE, iff.RS1, iff.RS2},
    "divi"  = []iff{iff.RDE, iff.RS1, iff.IMM},

    "andr"  = []iff{iff.RDE, iff.RS1, iff.RS2},
    "andi"  = []iff{iff.RDE, iff.RS1, iff.IMM},
    "orr"   = []iff{iff.RDE, iff.RS1, iff.RS2},
    "ori"   = []iff{iff.RDE, iff.RS1, iff.IMM},
    "norr"  = []iff{iff.RDE, iff.RS1, iff.RS2},
    "nori"  = []iff{iff.RDE, iff.RS1, iff.IMM},
    "xorr"  = []iff{iff.RDE, iff.RS1, iff.RS2},
    "xori"  = []iff{iff.RDE, iff.RS1, iff.IMM},
    "shlr"  = []iff{iff.RDE, iff.RS1, iff.RS2},
    "shli"  = []iff{iff.RDE, iff.RS1, iff.IMM},
    "asrr"  = []iff{iff.RDE, iff.RS1, iff.RS2},
    "asri"  = []iff{iff.RDE, iff.RS1, iff.IMM},
    "lsrr"  = []iff{iff.RDE, iff.RS1, iff.RS2},
    "lsri"  = []iff{iff.RDE, iff.RS1, iff.IMM},

    "push"  = []iff{iff.RS1},
    "pushi" = []iff{iff.IMM},
    "pushz" = []iff{iff.IMM},
    "pushc" = []iff{iff.IMM},
    "pop"   = []iff{iff.RDE},
    "enter" = []iff{},
    "leave" = []iff{},
    "reloc" = []iff{iff.RS1, iff.IMM},

    "ljal"  = []iff{iff.RS1, iff.IMM},
    "ljalr" = []iff{iff.RS1, iff.IMM, iff.RDE},
    "ret"   = []iff{},
    "retr"  = []iff{iff.RS1},

    "bra"   = []iff{iff.IMM},
    "beq"   = []iff{iff.IMM},
    "bez"   = []iff{iff.IMM},
    "blt"   = []iff{iff.IMM},
    "ble"   = []iff{iff.IMM},
    "bltu"  = []iff{iff.IMM},
    "bleu"  = []iff{iff.IMM},
    "bpe"   = []iff{iff.IMM},
    "bne"   = []iff{iff.IMM},
    "bnz"   = []iff{iff.IMM},
    "bge"   = []iff{iff.IMM},
    "bgt"   = []iff{iff.IMM},
    "bgeu"  = []iff{iff.IMM},
    "bgtu"  = []iff{iff.IMM},
    "bpd"   = []iff{iff.IMM},

    "jal"   = []iff{iff.IMM},
    "jalr"  = []iff{iff.IMM, iff.RDE},
}

native_instruction_formats := map[string]instruction_fmt{
    "nop"   = instruction_fmt.B,
    "int"   = instruction_fmt.B,
    "inv"   = instruction_fmt.M,
    "usr"   = instruction_fmt.B,

    "lli"   = instruction_fmt.F,
    "llis"  = instruction_fmt.F,
    "lui"   = instruction_fmt.F,
    "luis"  = instruction_fmt.F,
    "lti"   = instruction_fmt.F,
    "ltis"  = instruction_fmt.F,
    "ltui"  = instruction_fmt.F,
    "ltuis" = instruction_fmt.F,
    "ld"    = instruction_fmt.M,
    "lbs"   = instruction_fmt.M,
    "lb"    = instruction_fmt.M,
    "st"    = instruction_fmt.M,
    "stb"   = instruction_fmt.M,
    "swp"   = instruction_fmt.M,

    "addr"  = instruction_fmt.R,
    "addi"  = instruction_fmt.M,
    "adcr"  = instruction_fmt.R,
    "adci"  = instruction_fmt.M,
    "subr"  = instruction_fmt.R,
    "subi"  = instruction_fmt.M,
    "sbbr"  = instruction_fmt.R,
    "sbbi"  = instruction_fmt.M,
    "mulr"  = instruction_fmt.R,
    "muli"  = instruction_fmt.M,
    "divr"  = instruction_fmt.R,
    "divi"  = instruction_fmt.M,

    "andr"  = instruction_fmt.R,
    "andi"  = instruction_fmt.M,
    "orr"   = instruction_fmt.R,
    "ori"   = instruction_fmt.M,
    "norr"  = instruction_fmt.R,
    "nori"  = instruction_fmt.M,
    "xorr"  = instruction_fmt.R,
    "xori"  = instruction_fmt.M,
    "shlr"  = instruction_fmt.R,
    "shli"  = instruction_fmt.M,
    "asrr"  = instruction_fmt.R,
    "asri"  = instruction_fmt.M,
    "lsrr"  = instruction_fmt.R,
    "lsri"  = instruction_fmt.M,

    "push"  = instruction_fmt.F,
    "pushi" = instruction_fmt.F,
    "pushz" = instruction_fmt.F,
    "pushc" = instruction_fmt.F,
    "pop"   = instruction_fmt.F,
    "enter" = instruction_fmt.F,
    "leave" = instruction_fmt.F,
    "reloc" = instruction_fmt.F,

    "ljal"  = instruction_fmt.M,
    "ljalr" = instruction_fmt.M,
    "ret"   = instruction_fmt.F,
    "retr"  = instruction_fmt.F,
    "jal"   = instruction_fmt.J,
    "jalr"  = instruction_fmt.J,

    "bra"   = instruction_fmt.B,
    "beq"   = instruction_fmt.B,
    "bez"   = instruction_fmt.B,
    "blt"   = instruction_fmt.B,
    "ble"   = instruction_fmt.B,
    "bltu"  = instruction_fmt.B,
    "bleu"  = instruction_fmt.B,
    "bpe"   = instruction_fmt.B,
    "bne"   = instruction_fmt.B,
    "bnz"   = instruction_fmt.B,
    "bge"   = instruction_fmt.B,
    "bgt"   = instruction_fmt.B,
    "bgeu"  = instruction_fmt.B,
    "bgtu"  = instruction_fmt.B,
    "bpd"   = instruction_fmt.B,
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