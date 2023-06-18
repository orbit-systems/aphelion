package aas

import "core:os"
import str "core:strings"

// preprocessor
// takes the raw assembly and expands .inc, .def, and .mac

preprocess :: proc(t: string) -> (res: string) {

    res = str.expand_tabs(t, 4)

    {
        // hacky, redo later with the string scanner from the lexer
        included := make([dynamic]string)
        inc_seen := str.count(res, ".inc") != 0
        for inc_seen {
            inc_seen = false
            for l, index in str.split_lines(res) {
                if str.has_prefix(l, ".inc ") {
                    inc_seen = true
                    inc_path := l[6:(len(l)-1)]

                    inc_file, inc_readok := os.read_entire_file(inc_path)
                    if !inc_readok {
                        die("ERR: cannot read file at \"%s\"\n", inc_path)
                    }
                    
                    if !in_dynarr(included, inc_path) || flag_keep_dup_inc {
                        r, _ := str.replace(res, l, transmute(string) inc_file, 1)
                        res = r
                        append(&included, inc_path)
                        break
                    } else {
                        r, _ := str.remove(res, l, 1)
                        res = r
                    }
                }
            }
        }
    }
    

    return
}

in_dynarr :: proc(arr: [dynamic]string, str: string) -> bool {
    for i in arr {
        if str == i {
            return true
        }
    }
    return false
}

// todo make custom destructor for this bc this probably leaks memory
macro :: struct {
    name        : string,
    arguments   : [dynamic]string,
    body        : string,
}
