package aas

import "core:os"
import "core:fmt"

@(private="file")
die_exit_code :: 1

die :: proc{dief0, dief1}
dbg :: proc{dbgf0, dbgf1, dbgf2}
err :: fmt.eprint


dief0 :: proc(msg: string) {
    fmt.eprint(msg)
    os.exit(die_exit_code)
}
dief1 :: proc(msg: string, args: any) {
    fmt.eprintf(msg, args)
    os.exit(die_exit_code)
}


dbgf0 :: proc(msg: string) {
    if print_dbg do fmt.printf(msg)
}
dbgf1 :: proc(msg: string, args: any) {
    if print_dbg do fmt.printf(msg, args)
}
dbgf2 :: proc(msg: string, arg1, arg2: any) {
    if print_dbg do fmt.printf(msg, arg1, arg2)
}

print_help :: proc() {
    fmt.println("\nusage: aas (path) [flags]\n")
    fmt.println("-debug               print debug info")
    fmt.println("-out:[path]          set output path")
    fmt.println("-preprocess          only invoke preprocessor - expand macros, etc.")
    fmt.println("-ignore-ext          ignore file extension")
    fmt.println("-help                display this text")
}