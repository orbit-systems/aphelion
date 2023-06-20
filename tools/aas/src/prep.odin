package aas

import "core:os"
import str "core:strings"
import "core:unicode/utf8"

// preprocessor
// takes the raw assembly and expands .inc, .def, and .mac

preprocess :: proc(t: string) -> (res: string) {

    // expand tabs
    res = str.expand_tabs(t, 4)

    // .def
    {
        defined := make([dynamic]string)
        defer delete(defined)
        for str.contains(res, ".def") {
            for l, index in str.split_lines(res) {
                if !str.contains(l, ".def") {
                    continue
                }

                line_tokens : [dynamic]btoken
                tokenize(l, &line_tokens) // lmfao invoking the lexer on a single line actually works pretty well

                if line_tokens[0].value != ".def" || len(line_tokens) != 3 {
                    continue
                }

                if in_dynarr(defined, line_tokens[1].value) {
                    die("ERR [line %d]: already defined \"%s\"\n", index, line_tokens[1].value)
                }

                append(&defined, line_tokens[1].value)

                
                line_removed, _ := str.remove(res, l, 1)
                definitions_inserted, _ := str.replace_all(line_removed, line_tokens[1].value, line_tokens[2].value)
                res = definitions_inserted
                break   // restart definition search
                
            }
        }
    }

    // .inc
    {
        included := make([dynamic]string)
        defer delete(included)
        for str.contains(res, ".inc") {
            for l, index in str.split_lines(res) {
                if !str.contains(l, ".inc") {
                    continue
                }

                line_tokens : [dynamic]btoken
                tokenize(l, &line_tokens) // lmfao invoking the lexer on a single line actually works pretty well

                if line_tokens[0].value != ".inc" || len(line_tokens) != 2 {
                    continue
                }

                inc_path := line_tokens[1].value[1:len(line_tokens[1].value)-1]

                inc_file, inc_readok := os.read_entire_file(inc_path)
                if !inc_readok {
                    die("ERR [line %d]: cannot read file at \"%s\"\n", index, inc_path)
                }
                
                if !in_dynarr(included, inc_path) || flag_keep_dup_inc {
                    r, _ := str.replace(res, l, transmute(string) inc_file, 1)
                    res = r
                    append(&included, inc_path)
                    break
                } else {
                    r, _ := str.remove(res, l, 1)
                    res = r
                    break
                }
            }
        }
    }
    

    return
}

// replace_word_all :: proc(s, key, value: string) -> string {
//     if !str.contains(s, key) {
//         return ""
//     }

//     new_s := s
//     last_found := 0
//     for str.contains(new_s[last_found:], key) {
//         start_loc := str.index(new_s[last_found:], key)
//         end_loc := start_loc + len(key) - 1
//         last_found = end_loc + 1
        
//         if start_loc == 0 || is_separator(utf8.rune_at(new_s, start_loc-1)) &&
//            end_loc == top(new_s) || is_separator(utf8.rune_at(new_s, end_loc+1)) {
            
//         }


//     }
//     return new_s
// }

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
