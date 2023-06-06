package aas

// embedder
// takes the statement chain and writes the actual binary file

// get the instructions and fill the encoding values correctly according to each instruction
// todo integrate this with one of the parser stages

precode :: proc(stmt_chain: ^[dynamic]statement) {
    for stmt, stmt_index in stmt_chain^ {
        if stmt.kind != statement_kind.Instruction {
            continue
        }

        if !(stmt.name in native_instruction_args_to_fields) {
            die("ERR [line %d]: cannot retrieve instruction embedding information for \"%s\"", stmt.line, stmt.name)
        }

        args_to_fields := native_instruction_args_to_fields[stmt.name]
        for arg, arg_index in args_to_fields {
            switch arg {
            case iff.R1:  
                stmt_chain^[stmt_index].r1 = stmt.args[arg_index].value_int
            case iff.R2:  
                stmt_chain^[stmt_index].r2 = stmt.args[arg_index].value_int
            case iff.R3:  
                stmt_chain^[stmt_index].r3 = stmt.args[arg_index].value_int
            case iff.IMM: 
                stmt_chain^[stmt_index].imm = stmt.args[arg_index].value_int
            }
        }
    }
}

// write to binary
make_bin :: proc(stmt_chain: ^[dynamic]statement, imglen: int) -> []u8 {

    precode(stmt_chain)

    bin := make([]u8, imglen)

    for stmt, index in stmt_chain^ {

        switch stmt.kind {
        case statement_kind.Label:
            continue

        case statement_kind.Directive:
            bin[stmt.loc] = 0x44

        case statement_kind.Instruction:

            if !(stmt.name in native_instruction_formats) {
                die("ERR [line %d]: cannot retrieve instruction format for \"%s\"", stmt.line, stmt.name)
            }
            format := native_instruction_formats[stmt.name]

            ins : u32

            switch format {
            case instruction_fmt.R: ins = encode_r(stmt.r1, stmt.r2, stmt.r3, stmt.imm, stmt.opcode)
            case instruction_fmt.M: ins = encode_m(stmt.r1, stmt.r2, stmt.imm, stmt.opcode)
            case instruction_fmt.F: ins = encode_f(stmt.r1, stmt.func, stmt.imm, stmt.opcode)
            case instruction_fmt.J: ins = encode_j(stmt.r1, stmt.imm, stmt.opcode)
            case instruction_fmt.B: ins = encode_b(stmt.func, stmt.imm, stmt.opcode)
            }

            write_u32_le(&bin, stmt.loc, ins)
            

            //bin[stmt.loc] = cast(u8) stmt.opcode
        case statement_kind.Unresolved:
            die("wtf")
        }

    }

    
    return bin
}

write_u32_le :: proc(buf: ^[]u8, loc: int, val: u32) {
    buf^[loc]   = cast(u8) (val)
    buf^[loc+1] = cast(u8) (val >> 8)
    buf^[loc+2] = cast(u8) (val >> 16)
    buf^[loc+3] = cast(u8) (val >> 24)
}

encode_r :: proc(r1, r2, r3, imm, op: int) -> (bin: u32) {
    bin = bin | cast(u32) op            // embed opcode
    bin = bin | cast(u32) (imm << 8)    // embed imm
    bin = bin & cast(u32) 0xF_FFFF      // limit imm (ran into problems where the immediate value would overwrite everything that comes after it if it was negative)
    bin = bin | cast(u32) (r3 << 20)    // embed r3
    bin = bin | cast(u32) (r2 << 24)    // embed r2
    bin = bin | cast(u32) (r1 << 28)    // embed r1
    return
}

encode_m :: proc(r1, r2, imm, op: int) -> (bin: u32) {
    bin = bin | cast(u32) op            // embed opcode
    bin = bin | cast(u32) (imm << 8)    // embed imm
    bin = bin & cast(u32) 0xFF_FFFF     // limit imm
    bin = bin | cast(u32) (r2 << 24)    // embed r2
    bin = bin | cast(u32) (r1 << 28)    // embed r1
    return
}

encode_f :: proc(r1, func, imm, op: int) -> (bin: u32) {
    bin = bin | cast(u32) op            // embed opcode
    bin = bin | cast(u32) (imm << 8)    // embed imm
    bin = bin & cast(u32) 0xFF_FFFF     // limit imm
    bin = bin | cast(u32) (func << 24)  // embed func
    bin = bin | cast(u32) (r1 << 28)    // embed r1
    return
}

encode_j :: proc(r1, imm, op: int) -> (bin: u32) {
    bin = bin | cast(u32) op            // embed opcode
    bin = bin | cast(u32) (imm << 8)    // embed imm
    bin = bin & cast(u32) 0xFFF_FFFF    // limit imm
    bin = bin | cast(u32) (r1 << 28)    // embed r1
    return
}

encode_b :: proc(func, imm, op: int) -> (bin: u32) {
    bin = bin | cast(u32) op            // embed opcode
    bin = bin | cast(u32) (imm << 8)    // embed imm
    bin = bin & cast(u32) 0xFFF_FFFF    // limit imm
    bin = bin | cast(u32) (func << 28)  // embed r1
    return
}

instruction_fmt :: enum {
    R, M, F, J, B,
}

instruction_fmt_fields :: enum {
    R1, R2, R3, IMM,
}