#import "@preview/cetz:0.4.2"

#let title = "Aphelion Application Binary Interface Specification"
#let subtitle = "Version 6"
#let authors = (
  (name: "Seth Poulsen", email: "sp@sandwichman.dev"),
)

#set raw(syntaxes: "../../assets/aphel.sublime-syntax");
#let orbit_grey = rgb("f2f2f2")
#let orbit_red  = rgb("f80530")
#let grey_cell = table.cell(fill: orbit_grey)[]

#let code(content, width: 100%) = box(content, fill: orbit_grey, inset: 10pt, radius: 3pt, stroke: 0.5pt + luma(50%), width: width)

#let ccenter(a) = table.cell(a, align: center)
#let cc_set(a) = table.cell(text(a), align: center)

#let format64(..fields) = layout(ly => cetz.canvas(length: ly.width/64, {
    import cetz.draw: *;
  
    let notch_width = 0.15*2;
    let bitnum_size = 9pt;
    let name_size = bitnum_size;
    let fields = fields.pos();
    
    // draw text
    let bits = 64;
    for (name, bitwidth) in fields {
        if (name == "unused" or name.match("^grey-") != none) {
            rect((bits - bitwidth,0), (bits, 2), stroke: 0pt, fill: orbit_grey)
        }
        if bitwidth != 1 {
          content(
              anchor: "east", (bits - 0.1, 3),
              text(size: bitnum_size)[#{64 - bits}]
          );
          content(
              anchor: "west", (bits - bitwidth + 0.1, 3),
              text(size: bitnum_size)[#{64 + bitwidth - bits - 1}]
          );
        } else {
          content(
              anchor: "mid", (bits - 0.5, 3),
              text(size: bitnum_size)[#{64 - bits}]
          );
        }
        if (name.match(regex("^[01]+$")) == none) {
            if (name.match("grey-") == none) {
                content(
                    anchor: "mid", (bits - bitwidth/2, 1),
                    text(size: name_size)[#name]
                );
            } else {
                let realname = name.trim("grey-", at: start);
                content(
                    anchor: "mid", (bits - bitwidth/2, 1),
                    text(size: name_size)[#realname]
                );
            }
        } else {
            let n = 0;
            for char in name {
                content(
                    anchor: "mid", (bits - bitwidth + 1 + n, 0.5),
                    text(size: name_size)[#char]
                );
                n += 1;
            }
        }
        bits -= bitwidth;
    }

    // draw box and lines
    let bits = 64;
    for (name, bitwidth) in fields {
        bits -= bitwidth;
        for i in range(1, bitwidth) {
            line((bits + i, 0), (bits + i, notch_width), stroke: 0.5pt)
            line((bits + i, 2), (bits + i, 2-notch_width), stroke: 0.5pt)
        }
        if bits != 0 {
            line((bits, 0), (bits, 2), stroke: 0.5pt)
        }
    }
    rect((0,0), (64, 2), stroke: 0.5pt);
}))

#let format(..fields) = layout(ly => cetz.canvas(length: ly.width/32, {
    import cetz.draw: *;
  
    let notch_width = 0.15;
    let bitnum_size = 10pt;
    let name_size = bitnum_size;
    let fields = fields.pos();
    
    // draw text
    let bits = 32;
    for (name, bitwidth) in fields {
        if (name == "unused" or name == "unused(hidden)") {
            rect((bits - bitwidth,0), (bits, 1), stroke: 0pt, fill: orbit_grey)
        }
        content(
            anchor: "mid", (bits - 0.5, 1.5),
            text(size: bitnum_size)[#{32 - bits}]
        );
        if bitwidth != 1 {
            content(
                anchor: "mid", (bits - bitwidth + 0.5, 1.5),
                text(size: bitnum_size)[#{32 + bitwidth - bits - 1}]
            );
        }
        if (name.match(regex("^[01]+$")) == none) {
            if (name != "unused(hidden)") {
                content(
                    anchor: "mid", (bits - bitwidth/2, 0.5),
                    text(size: name_size)[#name]
                );
            }
        } else {
            let n = 0;
            for char in name {
                content(
                    anchor: "mid", (bits - bitwidth + 0.5 + n, 0.5),
                    text(size: name_size)[#char]
                );
                n += 1;
            }
        }
        bits -= bitwidth;
    }

    // draw box and lines
    let bits = 32;
    for (name, bitwidth) in fields {
        bits -= bitwidth;
        for i in range(1, bitwidth) {
            line((bits + i, 0), (bits + i, notch_width), stroke: 0.5pt)
            line((bits + i, 1), (bits + i, 1-notch_width), stroke: 0.5pt)
        }
        if bits != 0 {
            line((bits, 0), (bits, 1), stroke: 0.5pt)
        }
    }
    rect((0,0), (32, 1), stroke: 0.5pt);
}))


#let reloc_word(..fields) = layout(ly => cetz.canvas(length: ly.width/32, {
    import cetz.draw: *;


    
    // if (start != 0) {
    //     content(
    //         anchor: "east", (32, 1.5),
    //         text(size: 9pt)[0]
    //     );
    // }
    // if (end != 31) {
    //     content(
    //         anchor: "west", (0, 1.5),
    //         text(size: 9pt)[31]
    //     );
    // }

    let bitbounds_start = ();
    let bitbounds_end = ();

    for f in fields.pos() {
        let (start, end, name) = f;

        bitbounds_start.push(start);
        bitbounds_end.push(end);
    }
    bitbounds_start.push(0);
    bitbounds_end.push(31);

    bitbounds_start = bitbounds_start.dedup();
    bitbounds_end = bitbounds_end.dedup();

    for start in bitbounds_start {
        content(
            anchor: "east", (32-start, 1.5),
            text(size: 9pt)[#{start}]
        ); 
    }

    for end in bitbounds_end {
        content(
            anchor: "west", (32-end - 1, 1.5),
            text(size: 9pt)[#{end}]
        );
    }

    let hword = 0;
    for f in fields.pos() {
        let (start, end, name) = f;
    
        rect((32-end - 1,hword), (0, hword + 1), stroke: 0pt, fill: orbit_grey);
        rect((32-start,hword), (32, hword + 1), stroke: 0pt, fill: orbit_grey);
        
        for i in range(1, 32) {
            if (32 - i - 1 == end or 32 - i == start) {
              line((i, hword), (i, hword + 1), stroke: 0.5pt)
                continue;
            }
            
            line((i, hword), (i, hword + 0.15), stroke: 0.5pt)
            line((i, hword + 1), (i, hword + 1 - 0.15), stroke: 0.5pt)
        }
        
        rect((0, hword), (32, hword + 1), stroke: 0.5pt);
        
        content(
            anchor: "mid", (31.5-(start+end)/2, hword + 0.5),
            text(size: 9pt)[#name]
        );
        hword -= 1;
    }
}));

#set page(paper: "a4")
#set text(font: "Io Serif", lang: "en", size: 12pt)
#show raw: set text(weight: 600, fill: luma(0), font: "IBM Plex Mono", size: 11pt)
#set list(marker: [-])
#set heading(numbering: "1.1.")

#set document(author: authors.map(a => a.name), title: title)
#v(0.1fr)
#link("https://github.com/orbit-systems")[#align(right, image("../../assets/orbitsystems.svg", width: 40%))]
#v(9fr)
#text(2em, weight: "bold", title)
#text(1.7em, weight: "light", " " + subtitle)
#pad(
  top: 4em,
  right: 15%,
  grid(
    columns: (1fr,) * calc.min(3, authors.len()),
    gutter: 3em,
    ..authors.map(author => align(start)[
      #text(author.name, weight:"bold") \
       #link("mailto:"+author.email)[#author.email]
    ]),
  ),
)

#set par(justify: true, linebreaks: "optimized")

#pagebreak()
#set page(
  header:
  [
    #set text(0.9em)
    _*Aphelion Application Binary Interface Specification* Version 6_
    #h(1fr) #context counter(page).display()
    
    #v(-0.4em)
    #line(length: 100%)
  ],
  footer: [],
  numbering: "i",
)

#set underline(evade: true, stroke: 1pt, offset: 2pt)
#let ulink(url, content) = underline(strong(link(url)[#content]))
#set table(stroke: 0.5pt)

Typeface #ulink("https://github.com/orbit-systems/io")[Io Serif] created by Echo Heo for Orbit Systems documentation.

Typeface #ulink("https://github.com/IBM/plex")[IBM Plex Mono] used in monospace/code-snippet contexts.

#line()

#pagebreak()
#outline(
  depth: 2,
  indent: auto,
)

#set page(
  numbering: "1"
)
#counter(page).update(1)
#pagebreak()
= Introduction

This document specifies the elements of the standard Application Binary Interface (ABI) for the Aphelion ISA. Standard tools, system implementations, and software shall implement the ABI to build mutually-compatible Aphelion software and ensure a cohesive, reliable software ecosystem.

// For the purposes of this document, *unoptimized software* refers to software that can be analyzed in a standard way for 

// = Software Call Stack Layout
// The stack pointer *sp* and frame pointer *fp* must be *word* (8 byte) aligned.


= Register Convention

Aphelion has 32 general purpose registers (GPRs), each 64 bits wide.

#table(
  fill: (x, y) => if y == 0 {orbit_grey},
  columns: (auto, auto, auto, 1fr),
  [\#],     [Name],        [Preserved \ across calls?], [Purpose],
  [0],      [`zr`],        grey_cell, [Hardwired to zero],
  [1..6],   [`a0..a5`],    [No],      [Function arguments/returns],
  [7..20],  [`l0..l13`],   [Yes],     [Local variables],
  [21..26], [`t0..t5`],    [No],      [Temporary variables],
  [27],     [`tp`],        grey_cell, [Thread pointer],
  [28],     [`fp`],        [Yes],     [Frame pointer],
  [29],     [`sp`],        [Yes],     [Stack pointer],
  [30],     [`lp`],        [No],      [Link/return pointer],
  [31],     [`ip`],        grey_cell, [Instruction pointer]
)

Standard ABI-conforming software shall not modify the thread pointer `tp`.

= Stack Convention

At procedure entrance and exit, the stack pointer `sp` shall be 8-byte aligned.

The call stack grows downward, meaning that:
- A procedure shall not use memory at addresses greater than or equal to the value of `sp` _at procedure entrance_ for local value storage, with the exception of stack-allocated arguments.
- At any execution point in the procedure, it shall not access memory at addresses less than the value of `sp` _at that point_. 

In short, procedures may only use memory they have allocated (by decrementing `sp`) for local storage, with the exception of stack-allocated arguments.

Use of the frame pointer `fp` is optional and may be used by procedures as a standard call-preserved register.

#pagebreak()
= C/C++ Type Convention

Scalar types in C/C++ shall have size and alignment described in the following table:

#table(
  fill: (x, y) => if y == 0 {orbit_grey},
  columns: (auto, auto, auto),
  [Type],             [Size], [Alignment],
  [`bool`/`_Bool`],   [1], [1],
  [`char`],           [1], [1],
  [`short`],          [2], [2],
  [`int`],            [4], [4],
  [`long`],           [8], [8],
  [`long long`],      [8], [8],
  [`__int128`],       [16], [16],
  [`void*`],             [8], [8],
  [`_Float16`],       [2], [2],
  [`float`],          [4], [4],
  [`double`],         [8], [8],
  [`long double`],    [16], [16],
  [`_Complex float`],          [8], [4],
  [`_Complex double`],         [16], [8],
  [`_Complex long double`],    [32], [16],
)

Type `char` shall be unsigned.

Values of type `bool` shall always be either 0 (false) or 1 (true).

Values of type `_Float16` and `long double` shall be represented using the `binary16` and `binary128` formats respectively, defined in IEEE 754-2019.

Complex types shall have the same representation as the C structure below, where `T` is the real type component:

#code[```c
struct Complex {
    T real;
    T imaginary;
};
```]

#pagebreak()
= Calling Convention

The standard calling convention views procedure signatures as a list of typed parameters and a list of return values (called *returns* in this document).

== Value Conversion

All values in the procedure signature undergo *value conversion*, which turns the procedure signature's richly-typed parameters and returns into lists of 64-bit words to pass and return.

For the purposes of this conversion, `bool` shall be treated as a 1-bit unsigned integer and `T*` shall be treated as a 64-bit unsigned integer.

Integer arguments and returns with size less than 64 bits shall be sign-extended (for signed integers) or zero-extended (for unsigned integers) to 64 bits.

Integer arguments and returns with size greater than 64 bits shall be treated as an aggregate of the same size.

#show footnote.entry: set text(red)

Floating-point arguments and returns of any size shall be treated as an aggregate of the same size. 

Aggregate arguments and returns with size less than or equal to 8 bytes shall be passed as though they are an integer argument, with identical byte layout. Padding and unused bits are undefined.

Aggregate arguments and returns with size greater than 8 bytes and less than or equal to 16 bytes shall be treated as two consecutive integer arguments, where the lower 8 bytes are passed first. Internal padding bits and bits not taken up by the aggregate value are undefined.

Aggregate arguments larger than 16 bytes shall be replaced in the argument list by a pointer. The storage backing the pointer shall not be mutated except through that pointer (equivalent to `restrict` qualification in C). Note that the aggregate may be mutated through this pointer, so it is recommended that a copy of this value is made on the caller side, though this copy may be optimized out if it would make no possible observable effect (e.g. the aggregate has no later uses or live references).

Aggregate returns larger than 16 bytes shall be removed from the return list and a pointer to write the aggregate return value shall be inserted into the beginning of the argument list. The storage backing the pointer shall not be mutated except through that pointer (equivalent to `restrict` qualification in C).

// === Examples
// The following examples are written in pseudocode resembling the syntax of the Rust programming language.

// #code[```rs
// // Before:
// fn foo(a: i32, b: i64, c: i128) 
//    -> (d: i128);
   
// // After:
// fn foo(a: i64, b: i64, c_low: i64, c_high: i64) 
//    -> (d_low: i64, d_high: i64);
// ```]

#pagebreak()
== Value Passing

After value conversion, the procedure signature is a list of argument words and a list of return words. 

Note that in this section, `sp` refers to the value of the stack pointer _at procedure entrance_.

At procedure exit, the first 6 return words are placed in order in registers `a0` to `a5`. If there are more than 6 return words, the rest shall be placed at consecutive positive offsets from `sp`, starting from 0.

At procedure entrance, the first 6 argument words are placed in order in registers `a0` to `a5`. If there are more than 6 argument words, the rest shall be placed _after_ any stack-placed return words.

For example, if there are 8 argument words and 8 return words, the 7th return word would be placed at `sp + 0`, the 8th return word would be placed at `sp + 8`, the 7th argument word would be placed at `sp + 16`, and the 8th argument word would be placed at `sp + 24`.

All variadic argument words are placed on the stack as if there are no available registers, even if there are.

In the following table:
- $n_"arg"$ represents the number of argument words placed on the stack; 
- $n_"ret"$ represents the number of return words placed on the stack;

#table(
  fill: (x, y) => if y == 0 {orbit_grey},
  // columns: (auto, auto),
  columns: (1fr, 1fr),
  stroke: 0pt,
  table.hline(stroke: 0.5pt),
  table.vline(stroke: 0.5pt),
  [Position], 
  table.vline(stroke: 0.5pt),
  [Contents], 
  table.vline(stroke: 0.5pt, ),
  table.hline(stroke: 0.5pt),
  [
`sp + 8*(n_ret + n_arg - 1)`],  [Stack Argument Word $n_"arg" - 1$],
  [\...],                       [\...],
  [`sp + 8*(n_ret + 1)`],   [Stack Argument Word 1],
  [`sp + 8*(n_ret)`],   [Stack Argument Word 0],
  table.hline(stroke: 0.5pt),
  [`sp + 8*(n_ret - 1)`],           [Stack Return Word $n_"ret" - 1$],
  [\...],                           [\...],
  [`sp + 8`],                   [Stack Return Word 1],
  [`sp + 0`],                   [Stack Return Word 0],
  table.hline(stroke: 0.5pt),
)

Very few applications and languages take advantage of the multiple-return capabilities of the calling convention, so $n_"ret"$ will almost always be 0.

// #image("image.png")

// #align(center)[#layout(ly => cetz.canvas(length: ly.width/12, {
//     import cetz.draw: *;

//     let y = 3;
//     let boxwidth = 4;

//     // content((4, 0), anchor: "center")[
        
//     // ];
    
//     rect((0,y), (boxwidth, y + 1), stroke: 0.5pt);
//     content((boxwidth/2, y + 0.5), anchor: "center")[
//       Argument Word N
//     ];
//     y -= 1;
    
//     content((boxwidth/2, y + 0.72), anchor: "south")[
//       $dots$
//     ];
//     y -= 0.5;
    
//     rect((0,y), (boxwidth, y + 1), stroke: 0.5pt);
//     content((boxwidth/2, y + 0.5), anchor: "center")[
//       Argument Word 8
//     ];
//     y -= 1;
    
//     rect((0,y), (boxwidth, y + 1), stroke: 0.5pt);
//     content((boxwidth/2, y + 0.5), anchor: "center")[
//       Argument Word 7
//     ];
//     y -= 1;
    
//     rect((0,y), (boxwidth, y + 1), stroke: 0.5pt);
//     content((boxwidth/2, y + 0.5), anchor: "center")[
//       Return Word 8
//     ];
//     y -= 1;
    
//     rect((0,y), (boxwidth, y + 1), stroke: 0.5pt);
//     content((boxwidth/2, y + 0.5), anchor: "center")[
//       Return Word 7
//     ];
//     y -= 1;
// }))]

// == Examples

// Signature:

// #code[```rs
// fn foo(
//   a: i32, b: i64, c: i128,
//   d: *i64, e: BigStruct, f: i128,
// ) -> (
//   g: i32, h: i64, i: i128,
//   j: *i64, k: BigStruct, l: i128,
// );
// ```]

// After value conversion:

// #code[```rs
// fn foo(
//   k_return_ptr, 
//   a, b, c_low, c_high, 
//   d, e_ptr, fd_low, f_high : i64
// ) -> (
//   g, h, i_low, i_high, 
//   j, l_low, l_high: i64
// );
// ```]

// After value passing:

// #align(right)[#layout(ly => cetz.canvas(length: ly.width/16, {
//     import cetz.draw: *;

//     let y = 0;
    
//     rect((0,y), (8, y + 1), stroke: 0.5pt);
//     content((4, y + 0.5), anchor: "center")[
//       `f_high`
//     ];
//     y -= 1;
    
//     rect((0,y), (8, y + 1), stroke: 0.5pt);
//     content((4, y + 0.5), anchor: "center")[
//       `f_low`
//     ];
//     y -= 1;
    
//     rect((0,y), (8, y + 1), stroke: 0.5pt);
//     content((4, y + 0.5), anchor: "center")[
//       `e_ptr`
//     ];
//     y -= 1;
    
//     rect((0,y), (8, y + 1), stroke: 0.5pt);
//     content((4, y + 0.5), anchor: "center")[
//       `a`
//     ];
//     y -= 1;
    
//     rect((0,y), (8, y + 1), stroke: 0.5pt);
//     content((4, y + 0.5), anchor: "center")[
//       `e_return_ptr`
//     ];
//     y -= 1;
    
//     rect((0,y), (8, y + 1), stroke: 0.5pt);
//     content((4, y + 0.5), anchor: "center")[
//       `f_high` (return slot)
//     ];
//     y -= 1;
//     // arrow()

//     // line((-0.1,y + 1), (-0.7, y + 1), stroke: 0.5pt)
//     // line((-0.1,y + 1), (-0.3, y + 0.8), stroke: 0.5pt)
//     // line((-0.1,y + 1), (-0.3, y + 1.2), stroke: 0.5pt)
// }))]

#pagebreak()
= Linker Relocations

Aphelion linkers, object files, and assemblers must support different types of *linker relocations* to be able to manipulate and relocate assembled code effectively.

Relocations must support signed addends of at least 16 bits.

In this section:
- *S* refers to the value of the symbol associated with the relocation.
- *A* refers to the addend associated with the relocation.
- *P* refers to the address of the relocation itself.

== WORD
This relocation places the expression `S + A` into an aligned 64-bit word at `P`.

This relocation must be aligned to 8 bytes.

== WORD_UNALIGNED
This relocation places the expression `S + A` into an unaligned 64-bit word at `P`.

#box[
== CALL

This relocation corresponds to a `call` assembly sequence. This places:
- `(S + A - P) >> 16` into bits 16..31 of the instruction at `P`; 
- `(S + A - P) >> 2` into bits 18..31 of the instruction at `P + 4`;

This relocation must be aligned to 4 bytes.

// #reloc_word((16, 31, "(S + A - P) >> 16"))
// #reloc_word((18, 31, "(S + A - P) >> 2"))

#reloc_word(
  (16, 31, "(S + A - P) >> 16"), 
  (18, 31, "(S + A - P) >> 2")
)]

#box[
== FCALL

This relocation corresponds to an `fcall` assembly sequence. This places:
- `(S + A) >> 48` into bits 16..31 of the instruction at `P`;
- `(S + A) >> 32` into bits 16..31 of the instruction at `P + 4`;
- `(S + A) >> 16` into bits 16..31 of the instruction at `P + 8`;
- `(S + A) >> 2` into bits 18..31 of the instruction at `P + 12`;

This relocation must be aligned to 4 bytes.

#reloc_word(
  (16, 31, "(S + A) >> 48"), 
  (16, 31, "(S + A) >> 32"), 
  (16, 31, "(S + A) >> 16"), 
  (18, 31, "(S + A) >> 2")
)]

#box[
== LI

This relocation corresponds to an `li` assembly sequence. This places:
- `(S + A) >> 48` into bits 16..31 of the instruction at `P`;
- `(S + A) >> 32` into bits 16..31 of the instruction at `P + 4`;
- `(S + A) >> 16` into bits 16..31 of the instruction at `P + 8`;
- `(S + A)` into bits 16..31 of the instruction at `P + 12`;

This relocation must be aligned to 4 bytes.

#reloc_word(
  (16, 31, "(S + A) >> 48"), 
  (16, 31, "(S + A) >> 32"), 
  (16, 31, "(S + A) >> 16"), 
  (16, 31, "(S + A)")
)]

#pagebreak()
= Glossary

- *word*: The size of a register. 8 bytes, 64 bits.
- *half-word*: 4 bytes, 32 bits.
- *quarter-word*: 2 bytes, 16 bits.
- *(linker) relocation*: An object file entry that inserts/replaces a value in a section such that symbol values that change during linking are accurately updated where they are referenced.
