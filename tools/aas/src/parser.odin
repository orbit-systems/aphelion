package aas

import "core:fmt"
import "core:strings"
import "core:unicode/utf8"

// parser
// converts basic tokens into instruction chain, build symbol table, check for syntax errors

construct_tree :: proc(tree: ^[dynamic]tree_node) {
    
}

check_invalid :: proc() {

}


chain_node :: struct {
    kind        : chain_node_kind,  // what kind of node it is
    value_int   : int,              // int value
    value_str   : string,           // string value, if needed
    arg1        : ^chain_node,      // argument 1
    arg2        : ^chain_node,      // argument 2
    arg3        : ^chain_node,      // argument 3
    size        : int,              // size in bytes - labels are 0, instructions are 4, etc.
    pos         : int,              // position in binary file
}

chain_node_kind :: enum {
    Instruction,
    Directive,
    Label,
    Register,
    
    String,
    Integer,

    Unresolved,
}