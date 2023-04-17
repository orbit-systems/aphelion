# ╭────────────╮
# │  aas v0.1  │ (C)2023 Seth Poulsen
# ╰────────────╯

import std/strutils, std/os, std/parseopt
import lexer

var LoadPath: string
var StorePath: string

proc loadArguments() = 
    var p = initOptParser(commandLineParams().join(" "))
    while true:
        p.next()
        case p.kind
            of cmdEnd:
                break
            of cmdArgument:
                if LoadPath == "":
                    LoadPath = p.key
                else:
                    StorePath = p.key
            else: 
                discard

proc main() =

    loadArguments()
    let inputASM = readFile(LoadPath)

    runlexer(inputASM)
    echo ""
    

main()