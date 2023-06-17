package aas

// preprocessor
// takes the token chain and resolves macros, definitions
// * currently not working in any capacity - ill probably have to redo this

preprocess :: proc(t: ^[dynamic]btoken) {
    resolve_macros(t)
}

resolve_def :: proc(token_chain: ^[dynamic]btoken) {

    def_table := make(map[string]btoken)

    // collect definitions
    this_def_name := false
    for tok in token_chain {
        if tok.kind == btoken_kind.Directive && tok.value == ".def" {
            this_def_name = true
            continue
        }

        // if this_def_name {
            
        // }
    }

}

resolve_macros :: proc(token_chain: ^[dynamic]btoken) {

    macro_table : [dynamic]macro

    // collect macros
    inside_macro_name := false
    inside_macro_args := false
    inside_macro_body := false
    def_start         := -2     // also keeps track of whether macros still remain in the file or not
    for def_start != -1 {
        def_start = -1
        for tok, index in token_chain {
        
            if tok.kind == btoken_kind.Directive && tok.value == ".mac" {
                inside_macro_name = true
                def_start = index
                append(&macro_table, macro{})
                continue
            }

            if inside_macro_name {
                macro_table[top(&macro_table)].name = btoken{btoken_kind.Instruction, tok.value}
                inside_macro_name = false
                inside_macro_args = true
                continue
            }

            if inside_macro_args {
                if tok.kind != btoken_kind.Newline {
                    append(&(macro_table[top(&macro_table)].arguments), tok)
                } else {
                    inside_macro_args = false
                    inside_macro_body = true
                }
                continue
            }

            if inside_macro_body {
                if tok.kind == btoken_kind.Directive && tok.value == ".endmac" {
                    inside_macro_body = false
                    remove_range(token_chain, def_start, index+1)
                    break
                } else {
                    append(&(macro_table[top(&macro_table)].body), tok)
                }
            }
        }
    }

    // we do a little checking its called we do a little checking (i am deranged)
    for mac1, index in macro_table {
        for mac2 in macro_table[(index+1):] {
            if mac1.name == mac2.name {
                die("ERR: macro redefined \"%s\"", mac1.name.value)
            }
        }
    }

    // replace macro instances
    for mac in macro_table {
        for tok in token_chain^ {
            if tok.kind == btoken_kind.Instruction && tok.value == mac.name.value {
                //how to actually fucjking do this im too tired to figure out but the framework is there
            }
        }
    }

    //dbg("\n%#v\n", macro_table)

    delete(macro_table)
}

// todo make custom destructor for this bc this probably leaks memory
macro :: struct {
    name        : btoken,
    arguments   : [dynamic]btoken,
    body        : [dynamic]btoken,
}
