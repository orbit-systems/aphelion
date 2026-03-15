#import "@preview/cetz:0.4.2"

#let title = "Aphelion Instruction Set Architecture Specification"
#let subtitle = "Version 6"
#let authors = (
    (name: "Seth Poulsen", email: "sp@sandwichman.dev"),
    (name: "Kayla Silk-Corke", email: "kaylasilkc@gmail.com"),
    (name: "Krzysztof Wolicki", email: "der.teufel.mail@gmail.com"),
    (name: "Nick Borș", email: "nick@nickbors.cc"),
)

#set raw(syntaxes: "../../assets/aphel.sublime-syntax");
#let orbit_grey = rgb("f2f2f2")
#let orbit_red  = rgb("f80530")
#let grey_cell = table.cell(fill: orbit_grey)[]

#let code(content) = box(content, fill: orbit_grey, inset: 10pt, radius: 3pt, stroke: 0.5pt + luma(50%), width: 100%)

#let unused = table.cell(fill: orbit_grey)[unused]
#let reserved = table.cell(fill: orbit_grey)[reserved]

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
        _*Aphelion Instruction Set Architecture Specification* Version 6_
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

_Revision 1: September 17, 2025_

_Revision 2: September 25, 2025_

_Revision 3: December 23, 2025_

_Revision 4: January 22, 2026_

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

Aphelion is a little-endian, 64-bit RISC-like architecture focused on efficient, dense, data-driven execution.

= General Purpose Registers

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

Zero register `zr` is always zero and is unaffected by writes. Instruction pointer `ip` is unaffected by direct writes, but can be modified through control flow instructions.

Link pointer `lp`, stack pointer `sp`, frame pointer `fp`, and thread pointer `tp` are only considered special on the ABI level (see Aphelion ABI Specification).

Instruction pointer `ip` can only be modified through explicit control-flow instructions. Any writes to `ip` as if it were a standard GPR are ignored. `ip`'s main purpose as a GPR, and not a control register, is to facilitate simple position-independent code.

#box[
= Control Registers

Control registers, each 64 bits wide, are used to configure various LP-specific operations and modes. They are only accessible in kernel mode through the `sctrl` and `lctrl` instructions. Storing to read-only control registers with `sctrl` has no effect.

#table(
    columns: (auto, auto, 1fr),
    fill: (x, y) => if y == 0 {orbit_grey},
    "#", "Name", "Description",
    "0..15", `int0..int15`, [Interrupt handler pointers. Bits 0..1 of addresses stored here are hardwired to 0.],
    "16", `intip`, [Interrupt instruction pointer. When an interrupt is triggered, this is set to the value of `ip` at the time of the trigger. Used by the `iret` instruction to return from an interrupt handler or to jump into user mode. Bits 0..1 of addresses stored here are hardwired to 0.],
    "17", `intval`, [Any value or address relevant to an interrupt. When an `BUS*`, `ACCESS*`, `UALIGN*`, or `VATFAIL` interrupt trigger, the target address of the memory access that caused it will be stored here.],
    "18", `intpte`, [On an `ACCESS*` interrupt when using virtual addressing, this contains the most recent page table entry read by the processor.],
    "19", `intcause`, [The cause of the most recent interrupt. Read-only.],
    "20", `kptp`, [Page table pointer for virtual addressing in kernel mode. Bits 0..11 of addresses stored here are hardwired to 0.],
    "21", `uptp`, [Page table pointer for virtual addressing in user mode. Bits 0..11 of addresses stored here are hardwired to 0.],
    "22", `stat`, [Status register. Described in full below.],
    "23", `intstat`, [Interrupt status register. When an interrupt is triggered, `stat` is copied into `intstat`. The `iret` instruction copies `intstat` back into `stat`.],
    "24", `id`, [A unique ID for this LP in a multiprocessor system. Must be 0 in a single-processor system. Read-only.]
)]

All control register codes from 0 through 511 are reserved for use by the standard. Control register codes beyond this reserved region may be used for implementation-specific LP configuration.

=== STAT - Status Register

The control register `stat` is a set of bit fields that indicate and control the LP's state:

#table(
    columns: (auto, auto, 1fr),
    fill: (x, y) => if y == 0 {orbit_grey},
    [Bit(s)], [Name], [Description],
    [0], [E], [External Interrupts are enabled.],
    [1], [U], [User mode is enabled.],
    [2], [V], [Virtual address translation is enabled.],
)

Undefined bits are currently reserved for use by the standard.

#pagebreak()
= System Interaction

== Reset State
Upon reset or initial power-on/boot, each LP shall: 
- Set all general-purpose registers (except `ip`) and all standard control registers to 0; 
- Set `ip` to an implementation-defined reset vector. This reset vector is usually inside the System-Reserved Region (see Aphelion System Environment Specification);
- Set the lock state to unlocked (see @memory-model-llsc);

#pagebreak()
= Interrupts <interrupts>

Interrupts are signals that can disrupt the normal flow of computation.
Interrupts can be internal (caused by the LP) or external (caused by the system environment).

Internal interrupts act only on the LP that triggered them. They may be triggered through the execution of special instructions, such as `syscall` and `breakpt`, but are most often generated through invalid execution of some kind, such as an unaligned memory access, virtual address translation failure, access violation, or execution of an invalid operation.

Internal interrupts generated during the execution of an instruction will cause the instruction to stop executing and prevent further side effects of the instruction, such as register write-backs or memory accesses.

External interrupts are generated by the surrounding system, usually by IO devices. They can be deferred and may only trigger during the execution of an instruction.

When an interrupt triggers, all previously executed instructions must complete in order, including incomplete or re-ordered memory operations. If re-ordered memory operations cause an interrupt of their own, the interrupt that triggers earliest in program order takes priority and replaces the current interrupt trigger.

== Handling Interrupts

To handle an interrupt: 
+ Register `ip` is saved to control register `intip` and control register `stat` is saved to `intstat`.
+ Register `ip` is set to the value of a handler control register (`int0..int15`) based on the interrupt cause.
+ This LP is put into kernel mode with external interrupts disabled;
+ This LP's lock state is unlocked;

#box[
== Interrupt Causes

#table(
  columns: (auto, auto, 1fr),
  fill: (x, y) => if y == 0 {orbit_grey},
  "#", "Name", "Description",
  "0", `EXTERNL`, "External (IO) interrupt.",
  "1", `BREAKPT`, "Debugger breakpoint.",
  "2", `SYSCALL`, "System call.",
  "3", `INVALID`, "Invalid operation has been loaded and executed. This is either an operation that does not exist, exists but with invalid arguments, or exists but is not permitted in the current mode.",
  "4", `BUSR`,    "Bus fault while reading from memory.",
  "5", `BUSW`,    "Bus fault while writing to memory.",
  "6", `BUSX`,    "Bus fault while fetching code from memory.",
  "7", `ACCESSR`, "Access violation while reading from memory.",
  "8", `ACCESSW`, "Access violation while writing to memory.",
  "9", `ACCESSX`, "Access violation while fetching code from memory.",
  "10", `UALIGNR`, "Unaligned access while reading from memory.",
  "11", `UALIGNW`, "Unaligned access while writing to memory.",
  "12", `UALIGNX`, "Unaligned access while fetching code from memory.",
  "13", `VATFAIL`, "Virtual address translation failed due to a misconfigured/invalid page table. Access violations during address translation are converted into this interrupt code.",
  "14..15", grey_cell, "Reserved."
)]

#pagebreak()
= Memory Model <memory-model>
  
== Weak Consistency <memory-model-consistency>

Aphelion follows a *weak consistency* memory model. For any two memory accesses X and Y, where X is before Y in program order, X may be reordered after Y in the global memory order if:
- There is no memory location overlap between X and Y;
- There is no corresponding fence instruction between X and Y;

In the weak model, fence instructions provide synchronization. Memory operations cannot be reordered before or after a corresponding `fence` instruction.

== LL/SC and Atomics <memory-model-llsc>

Aphelion provides special load-lock (LL) and store-conditional (SC) instructions that can be used to make arbitrary computation atomic.

Each LP has a "lock state," which may be either locked or unlocked. The lock state additionally comprises a memory location and a width. In the syntax of the Rust programming language, it may be represented like so:

#code[```rust
struct Lock {
  locked: bool,
  width: u8,
  address: u64,
}
```]

Loading with an LL instruction updates the LP's lock state with the location and width of that load and "locks" it. If any LP stores to the range of memory covered by another LP's lock state (including its own), that lock state becomes unlocked. Additionally, this LP's lock state will become unlocked if any of these events happen on this LP:
- An interrupt occurs;
- An `iret` instruction executes (returning from an interrupt or entering user mode);
- A cache management instruction executes;

Storing with an SC instruction succeeds if and only if the LP's lock state is locked and the location and width of the store correspond to the location and width of the lock state.

Implementations are permitted use cache state as a simple and performant heuristic for modification, e.g. an external store to a memory location may cause locked locations in the same cache block to unlock.

Note that LL and SC instructions are _not_ fences and adhere to the same weak consistency rules as traditional loads and stores.

== Caches and Coherency <memory-model-cache>

Aphelion uses a split cache architecture, using separated *data cache* (d-cache) for data loads/stores and *instruction cache* (i-cache) for instruction fetches. The caches function according to these rules:
- Each LP's d-cache is required to be fully coherent with other d-caches in the system. When a store from any LP executes in the global memory order, its effects must be visible to all LPs and external memory. External stores (e.g. from memory mapped devices) are not required to be immediately visible in d-cache. D-cache may need to be manually invalidated for external modifications to be visible.
- I-cache is not required to be automatically coherent with d-cache or main memory. When a block is not present in i-cache, it may be loaded either from main memory or from a present d-cache block. This means that external stores may not be seen by i-cache if the corresponding d-cache is not also invalidated.

Cache blocks are 64 bytes in width.

== Virtual Address Translation

Aphelion structures memory into 4 KiB ($2^12$B) chunks called *pages*. Using virtual address translation (VAT), the arbitrary platform-specific structure of physical memory can be reorganized into a consistent layout, and user mode programs can be isolated from kernel mode data.

When VAT is enabled, instructions that access memory will always attempt to translate addresses, even if the operation is not successful (e.g. store-conditional) or is treated as a no-op (e.g. cache management on cache-less systems).

Virtual addresses are broken into these individual fields:

// #table(
//   columns: (16fr, 9fr, 9fr, 9fr, 9fr, 12fr),
//   fill: (x, y) => if y == 0 {orbit_grey},
//   "63..48", "47..39", "38..30", "29..21", "20..12", "11..0",
//    table.cell("sign-extended", fill: orbit_grey),
//    [`i0`], [`i1`], [`i2`], [`i3`], [*offset*],
// )

#format64(
  ("offset", 12),
  ("i3", 9),
  ("i2", 9),
  ("i1", 9),
  ("i0", 9),
  ("grey-sign-extended", 16),
)


A four-level page table is then used, where *i0* is the index into the first-level page table (located in physical memory at either `uptp` or `kptp`), *i1* is the index into the second-level page table, etc., and *offset* is the offset into the final page where the access occurs.

Aphelion processors may choose to implement translation cache mechanisms to prevent page table walks on every virtual access. If implemented, storing to `kptp` and `uptp` has special properties:
- Storing to `kptp` will invalidate translation cache entries associated with the previous kernel mode page table.
- Storing to `uptp` will invalidate translation cache entries associated with the previous user mode page table.

Note that kernel mode and user mode translation cache entries are invalidated separately.

Each table is a full page and contains 512 page table entries (PTEs), each 8 bytes long, broken into these bit fields:

// #table(
//   columns: (22fr, 6fr, 1fr, 1fr, 1fr),
//   fill: (x, y) => if y == 0 {orbit_grey},
//   "63..12", "11..3", ccenter[2], ccenter[1], ccenter[0],
//   [`NEXT`], unused, ccenter[`X`], ccenter[`W`], ccenter[`V`],
// )

#format64(
  ("v", 1),
  ("w", 1),
  ("x", 1),
  ("unused", 9),
  ("next", 52),
)


#box[

Where each bit field is defined as follows:

#table(
  columns: (auto, 1fr),
  fill: (x, y) => if y == 0 {orbit_grey},
  [Name], [Description],
  [*v*], [When set, this entry is valid.],
  [*w*], [When set, this page is writeable. Ignored until the final table.],
  [*x*], [When set, this page is executable. Ignored until the final table.],
  [*next*], [These are the upper bits of the physical address of the next page table level or target page.]
)]

Any unused bits are ignored and may be used for software-specific information.

Virtual address translation will trigger an `ACCESS*` interrupt when:
- Bits 63..48 of the virtual address do not match bit 47;
- The *v* bit is not set in a page table entry used in translation;
- The *w* or *x* bits are not set in the final page table entry when writing data or fetching instructions respectively;


Virtual address translation will trigger a `VATFAIL` interrupt when the LP fails to load a page table entry.

#box[
= Instructions

Instructions are always 32 bits in length. Instructions must always be loaded from addresses that are 4-byte aligned. 

Bits 0..1 specify the format the instruction follows. Bits 2..8 provide a format-specific opcode. Together, the lowest byte of an instruction uniquely identify it.

#table(
  columns: (auto, 4fr, 5fr, 5fr, 5fr, 5fr, 6fr, 1fr, 1fr),
  fill: (x, y) => if y == 0 or y == 1 {orbit_grey},
  table.cell("Fmt", rowspan: 2, align: horizon + center), 
  table.cell("Bits", colspan: 8, align: center),
  table.hline(stroke: 0pt),
  "31..28", "27..23", "22..18", "17..13", "12..8", "7..2", ccenter("1"), ccenter("0"),
  "A", table.cell("imm19", colspan: 4),             "r1", "opcode", cc_set("0"), cc_set("0"),
  "B", table.cell("imm14", colspan: 3),       "r2", "r1", "opcode", cc_set("0"), cc_set("1"),
  "C", table.cell("imm9",  colspan: 2), "r3", "r2", "r1", "opcode", cc_set("1"), cc_set("0"),
)]

#include "opcodes.typ"

#pagebreak()

The following instruction implementations are written in pseudocode resembling the syntax of the Rust programming language.




// #let fmt_a(imm, r1, op) = table(
//   columns: (19fr, 5fr, 6fr, 1fr, 1fr),
//   fill: (x, y) => if y == 0 {orbit_grey},
//   "31..13", "12..8", "7..2", ccenter("1"), ccenter("0"),
//   imm, r1, op, cc_set("0"), cc_set("0"),
// )

// #let fmt_b(imm, r2, r1, op) = table(
//   columns: (14fr, 5fr, 5fr, 6fr, 1fr, 1fr),
//   fill: (x, y) => if y == 0 {orbit_grey},
//   "31..18", "17..13", "12..8", "7..2", ccenter("1"), ccenter("0"),
//   imm, r2, r1, op, cc_set("0"), cc_set("1"),
// )

// #let fmt_c(imm, r3, r2, r1, op) = table(
//   columns: (9fr, 5fr, 5fr, 5fr, 6fr, 1fr, 1fr),
//   fill: (x, y) => if y == 0 {orbit_grey},
//   "31..23", "22..18", "17..13", "12..8", "7..2", ccenter("1"), ccenter("0"),
//   imm, r3, r2, r1, op, cc_set("1"), cc_set("0"),
// )

#let fmt_a(imm, r1, op) = format(
  ("0", 1),
  ("0", 1),
  (op, 6),
  (r1, 5),
  (imm, 19),
)

#let fmt_b(imm, r2, r1, op) = format(
  ("1", 1),
  ("0", 1),
  (op, 6),
  (r1, 5),
  (r2, 5),
  (imm, 14),
)

#let fmt_c(imm, r3, r2, r1, op) = format(
  ("0", 1),
  ("1", 1),
  (op, 6),
  (r1, 5),
  (r2, 5),
  (r3, 5),
  (imm, 9),
)

== Memory Loads

#box[
=== LW - Load Word <LW>

#fmt_c("offset", "r3", "r2", "r1", "000100")
Load a 64-bit value into *r1* from the address given by the sum of *r2*, *r3*, and zero-extended *offset* shifted left by 3.

#code[```asm
lw r1, [r2 + r3 + 4088] ; offset argument is divided by 8
lw r1, [r2 + 4088]      ; r3 = zr
lw r1, [r2 + r3]        ; offset = 0
lw r1, [r2]             ; r3 = zr; offset = 0
```]
#code[```rust
let addr = gpr[r2] + gpr[r3] + zero_extend(offset) << 3;
gpr[r1] = mem_load64(addr);
```]
#line(length: 100%)
]

#box[
=== LH - Load Half-word <LH>
#fmt_c("offset", "r3", "r2", "r1", ("001100"))

Load a zero-extended 32-bit value into *r1* from the address given by the sum of *r2*, *r3*, and zero-extended *offset* shifted left by 2.

#code[```asm
lh r1, [r2 + r3 + 2044] ; offset argument is divided by 4
lh r1, [r2 + 2044]      ; r3 = zr
lh r1, [r2 + r3]        ; offset = 0
lh r1, [r2]             ; r3 = zr; offset = 0
```]
#code[```rust
let addr = gpr[r2] + gpr[r3] + zero_extend(offset) << 2;
gpr[r1] = zero_extend(mem_load_32(addr));
```]
#line(length: 100%)
]

#box[
=== LQ - Load Quarter-word <LQ>
#fmt_c("offset", "r3", "r2", "r1", ("010100"))
Load a zero-extended 16-bit value into *r1* from the address given by the sum of *r2*, *r3*, and zero-extended *offset* shifted left by 1.

#code[```asm
lq r1, [r2 + r3 + 1022] ; offset argument is divided by 2
lq r1, [r2 + 2044]      ; r3 = zr
lq r1, [r2 + r3]        ; offset = 0
lq r1, [r2]             ; r3 = zr; offset = 0
```]
#code[```rust
let addr = gpr[r2] + gpr[r3] + zero_extend(offset) << 1;
gpr[r1] = zero_extend(mem_load_16(addr));
```]
#line(length: 100%)
]

#box[
=== LB - Load Byte <LB>
#fmt_c("offset", "r3", "r2", "r1", ("011100"))
Load a zero-extended 8-bit value into *r1* from the address given by the sum of *r2*, *r3*, and zero-extended *offset*.

#code[```asm
lq r1, [r2 + r3 + 511]
lq r1, [r2 + 511]      ; r3 = zr
lq r1, [r2 + r3]       ; offset = 0
lq r1, [r2]            ; r3 = zr; offset = 0
```]
#code[```rust
let addr = gpr[r2] + gpr[r3] + zero_extend(offset);
gpr[r1] = zero_extend(mem_load_8(addr));
```]
#line(length: 100%)
]

#box[
=== LLW - Load-Lock Word <LLW>
#fmt_c("offset", "r3", "r2", "r1", ("100100"))

Load a 64-bit value into *r1* from the address given by the sum of *r2*, *r3*, and zero-extended *offset* shifted left by 3, and lock that memory location.

#code[```asm
llw r1, [r2 + r3 + 4088] ; offset argument is divided by 8
llw r1, [r2 + 4088]      ; r3 = zr
llw r1, [r2 + r3]        ; offset = 0
llw r1, [r2]             ; r3 = zr; offset = 0
```]
#code[```rust
let addr = gpr[r2] + gpr[r3] + zero_extend(offset);
lock.locked = true;
lock.address = addr;
lock.width = 8;
gpr[r1] = mem_load64(addr);
```]
#line(length: 100%)
]


#box[
=== LLH - Load-Lock Half-word <LLH>
#fmt_c("offset", "r3", "r2", "r1", ("101100"))

Load a zero-extended 32-bit value into *r1* from the address given by the sum of *r2*, *r3*, and zero-extended *offset* shifted left by 2, and lock that memory location.

#code[```asm
llh r1, [r2 + r3 + 2044] ; offset argument is divided by 4
llh r1, [r2 + 2044]      ; r3 = zr
llh r1, [r2 + r3]        ; offset = 0
llh r1, [r2]             ; r3 = zr; offset = 0
```]
#code[```rust
let addr = gpr[r2] + gpr[r3] + zero_extend(offset);
lock.locked = true;
lock.address = addr;
lock.width = 4;
gpr[r1] = mem_load32(addr);
```]
#line(length: 100%)
]

#box[
=== LLQ - Load-Lock Quarter-word <LLQ>
#fmt_c("offset", "r3", "r2", "r1", ("110100"))

Load a zero-extended 16-bit value into *r1* from the address given by the sum of *r2*, *r3*, and zero-extended *offset* shifted left by 1, and lock that memory location.

#code[```asm
llq r1, [r2 + r3 + 1022] ; offset argument is divided by 2
llq r1, [r2 + 1022]      ; r3 = zr
llq r1, [r2 + r3]        ; offset = 0
llq r1, [r2]             ; r3 = zr; offset = 0
```]
#code[```rust
let addr = gpr[r2] + gpr[r3] + zero_extend(offset);
lock.locked = true;
lock.address = addr;
lock.width = 2;
gpr[r1] = mem_load16(addr);
```]
#line(length: 100%)
]

#box[
=== LLB - Load-Lock Byte <LLB>
#fmt_c("offset", "r3", "r2", "r1", ("111100"))

Load a zero-extended 8-bit value into *r1* from the address given by the sum of *r2*, *r3*, and zero-extended *offset*, and lock that memory location.

#code[```asm
llq r1, [r2 + r3 + 511]
llq r1, [r2 + 511]      ; r3 = zr
llq r1, [r2 + r3]       ; offset = 0
llq r1, [r2]            ; r3 = zr; offset = 0
```]
#code[```rust
let addr = gpr[r2] + gpr[r3] + zero_extend(offset);
lock.locked = true;
lock.address = addr;
lock.width = 1;
gpr[r1] = mem_load8(addr);
```]
#line(length: 100%)
]
== Memory Stores

#box[
=== SW - Store Word <SW>
#fmt_c("offset", "r3", "r2", "r1", ("000101"))
Store a 64-bit value from *r1* to the address given by the sum of *r2*, *r3*, and zero-extended *offset* shifted left by 3.

#code[```asm
sw [r2 + r2 + 4088], r1 ; offset argument is divided by 8
sw [r2 + 4088], r1      ; r3 = zr
sw [r2 + r3], r1        ; offset = 0
sw [r2], r1             ; r3 = zr; offset = 0
```]
#code[```rust
let addr = gpr[r2] + gpr[r3] + zero_extend(offset) << 3;
mem_store64(gpr[r1], addr);
```]
#line(length: 100%)
]

#box[
=== SH - Store Half-word <SH>
#fmt_c("offset", "r3", "r2", "r1", ("001101"))
Store the lower 32 bits of *r1* to the address given by the sum of *r2*, *r3*, and zero-extended *offset* shifted left by 2.

#code[```asm
sh [r2 + r2 + 4088], r1 ; offset argument is divided by 4
sh [r2 + 4088], r1      ; r3 = zr
sh [r2 + r3], r1        ; offset = 0
sh [r2], r1             ; r3 = zr; offset = 0
```]
#code[```rust
let addr = gpr[r2] + gpr[r3] + zero_extend(offset) << 2;
mem_store32(gpr[r1] as u32, addr);
```]
#line(length: 100%)
]

#box[
=== SQ - Store Quarter-word <SQ>
#fmt_c("offset", "r3", "r2", "r1", ("010101"))
Store the lower 16 bits of *r1* to the address given by the sum of *r2*, *r3*, and zero-extended *offset* shifted left by 1.

#code[```asm
sq [r2 + r2 + 1022], r1 ; offset argument is divided by 2
sq [r2 + 1022], r1      ; r3 = zr
sq [r2 + r3], r1        ; offset = 0
sq [r2], r1             ; r3 = zr; offset = 0
```]
#code[```rust
let addr = gpr[r2] + gpr[r3] + zero_extend(offset) << 1;
mem_store16(gpr[r1] as u16, addr);
```]
#line(length: 100%)
]

#box[
=== SB - Store Byte <SB>
#fmt_c("offset", "r3", "r2", "r1", ("011101"))
Store the lower 8 bits of *r1* to the address given by the sum of *r2*, *r3*, and zero-extended *offset*.

#code[```asm
sb [r2 + r2 + 511], r1
sb [r2 + 511], r1      ; r3 = zr
sb [r2 + r3], r1       ; offset = 0
sb [r2], r1            ; r3 = zr; offset = 0
```]
#code[```rust
let addr = gpr[r2] + gpr[r3] + zero_extend(offset);
mem_store8(gpr[r1] as u8, addr);
```]
#line(length: 100%)
]

#box[
=== SCW - Store-Conditional Word <SCW>
#fmt_c("offset", "r3", "r2", "r1", ("100101"))
Store *r1* to the address given by the sum of *r3* and zero-extended *offset* shifted left by 3, if the memory location is locked. Set *r2* to 1 if the store was successful, or 0 otherwise.

#code[```asm
scw r2, [r3 + 4088], r1 ; offset argument is divided by 8
scw r2, [r3], r1        ; offset = 0
```]
#code[```rust
let addr = gpr[r3] + zero_extend(offset) << 3;
if lock.locked && lock.addr == addr && lock.width == 8 {
  mem_store64(gpr[r1], addr);
  gpr[r2] = 1;
} else {
  gpr[r2] = 0;
}
```]
#line(length: 100%)
]

#box[
=== SCH - Store-Conditional Half-word <SCH>
#fmt_c("offset", "r3", "r2", "r1", ("101101"))
Store the lower 32 bits of *r1* to the address given by the sum of *r3* and zero-extended *offset* shifted left by 2, if the memory location is locked. Set *r2* to 1 if the store was successful, or 0 otherwise.

#code[```asm
sch r2, [r3 + 2044], r1 ; offset argument is divided by 4
sch r2, [r3], r1        ; offset = 0
```]
#code[```rust
let addr = gpr[r3] + zero_extend(offset) << 2;
if lock.locked && lock.addr == addr && lock.width == 4 {
  mem_store32(gpr[r1] as u32, addr);
  gpr[r2] = 1;
  lock.locked = false;
} else {
  gpr[r2] = 0;
}
```]
#line(length: 100%)
]

#box[
=== SCQ - Store-Conditional Quarter-word <SCQ>
#fmt_c("offset", "r3", "r2", "r1", ("110101"))
Store the lower 16 bits of *r1* to the address given by the sum of *r3* and zero-extended *offset* shifted left by 1, if the memory location is locked. Set *r2* to 1 if the store was successful, or 0 otherwise.

#code[```asm
scq r2, [r3 + 1022], r1 ; offset argument is divided by 2
scq r2, [r3], r1        ; offset = 0
```]
#code[```rust
let mut addr = gpr[r3] + zero_extend(offset);
if lock.locked && lock.addr == addr && lock.width == 2 {
  mem_store16(gpr[r1] as u16, addr);
  gpr[r2] = 1;
  lock.locked = false;
} else {
  gpr[r2] = 0;
}
```]
#line(length: 100%)
]

#box[
=== SCB - Store-Conditional Byte <SCB>
#fmt_c("offset", "r3", "r2", "r1", ("111101"))
Store the lower 8 bits of *r1* to the address given by the sum of *r3* and zero-extended *offset*, if the memory location is locked. Set *r2* to 1 if the store was successful, or 0 otherwise.

#code[```asm
scb r2, [r3 + 1022], r1 ; offset argument is divided by 2
scb r2, [r3], r1        ; offset = 0
```]
#code[```rust
let mut addr = gpr[r3] + zero_extend(offset);
if lock.locked && lock.addr == addr && lock.width == 1 {
  mem_store8(gpr[r1] as u8, addr);
  gpr[r2] = 1;
  lock.locked = false;
} else {
  gpr[r2] = 0;
}
```]
#line(length: 100%)
]

== Memory Effects

#box[
=== FENCE - Memory Fence <FENCE>
// #fmt_a("imm19", "unused", ("000100"))
#format(
  ("0", 1),
  ("0", 1),
  ("000100", 6),
  ("unused", 5),
  ("l", 1),
  ("s", 1),
  ("unused", 17),
)

Before the next instruction executes, ensure that previous memory operations on this LP have completed in the global memory order and prevent memory operations further in program order from completing. If bit 0 of *imm19* (notated as *l*) is set, fence against load operations. If bit 1 of *imm19* (notated as *s*) is set, fence against store operations.

#code[```asm
fence   ; s = 1; l = 1
fence.s ; s = 1; l = 0
fence.l ; s = 0; l = 1
```]
#code[```rust
if l == 1 {
  ensure_loads_complete();
}
if s == 1 {
  ensure_stores_complete();
}
```]
#line(length: 100%)
]

#box[
=== CINVAL - Invalidate Cache <CINVAL>
// #fmt_a("imm19", "r1", ("001100"))
#format(
  ("0", 1),
  ("0", 1),
  ("001100", 6),
  ("r1", 5),
  ("d", 1),
  ("i", 1),
  ("m", 2),
  ("unused", 15),
)


Invalidate a region of cache.

If bit 0 of *imm19* (notated as *d*) is set, invalidate data cache. \
If bit 1 of *imm19* (notated as *i*) is set, invalidate instruction cache. \
If bits 2..3 of *imm19* (notated as *m*) are equal to:
- 0, invalidate only the cache block containing the address in *r1*.
- 1, invalidate all cache blocks associated with the page containing the address in *r1*.
- 2, invalidate the entire cache on this LP. The value of *r1* is unused.
- 3, trigger an `INVALID` interrupt.

This operation unlocks this LP's lock state.

#code[```asm
cinval.block   r1 ; m = 0; i = 1; d = 1
cinval.page    r1 ; m = 1; i = 1; d = 1
cinval.all        ; m = 2; i = 1; d = 1; r1 = zr
cinval.i.block r1 ; m = 0; i = 1; d = 0
cinval.i.page  r1 ; m = 1; i = 1; d = 0
cinval.i.all      ; m = 2; i = 1; d = 0; r1 = zr
cinval.d.block r1 ; m = 0; i = 0; d = 1
cinval.d.page  r1 ; m = 1; i = 0; d = 1
cinval.d.all      ; m = 2; i = 0; d = 1; r1 = zr
```]
#code[```rust
let addr = gpr[r1];
lock.locked = false;
match m {
  0 => invalidate_block(addr, d, i),
  1 => invalidate_page(addr, d, i),
  2 => invalidate_all(d, i),
  3 => trigger_interrupt(INVALID),
}```]
#line(length: 100%)
]

#box[
=== CFETCH - Fetch Cache <CFETCH>
// #fmt_a("imm19", "r1", ("010100"))
#format(
  ("0", 1),
  ("0", 1),
  ("010100", 6),
  ("r1", 5),
  ("l", 1),
  ("s", 1),
  ("i", 1),
  ("unused", 16),
)

Pre-fetch a cache block for a memory operation in the near future. Note that this does not guarantee the cache block is fresh from memory, as it will not invalidate and reload blocks that are already present in cache.

If bit 0 of *imm19* (notated as *l*) is set, fetch the cache block containing the address in *r1* for a data load operation.

If bit 1 of *imm19* (notated as *s*) is set, fetch the cache block containing the address in *r1* for a data store operation.

If bit 2 of *imm19* (notated as *i*) is set, fetch the cache block containing the address in *r1* for an instruction fetch operation.

This operation unlocks this LP's lock state.

#code[```asm
cfetch.l   r1 ; l = 1, s = 0, i = 0
cfetch.s   r1 ; l = 0, s = 1, i = 0
cfetch.i   r1 ; l = 0, s = 0, i = 1
cfetch.ls  r1 ; l = 1, s = 1, i = 0
cfetch.li  r1 ; l = 1, s = 0, i = 1
cfetch.si  r1 ; l = 0, s = 1, i = 1
cfetch.lsi r1 ; l = 1, s = 1, i = 1
```]
#code[```rust
let addr = gpr[r1];
lock.locked = false;
fetch_block(addr, l, s, i);

```]
#line(length: 100%)
]


== Arithmetic

#box[
=== SSI - Set Shifted Immediate <SSI>
// #fmt_a("imm19", "r1", ("000010"))
#format(
  ("0", 1),
  ("0", 1),
  ("000010", 6),
  ("r1", 5),
  ("c", 1),
  ("sh", 2),
  ("val", 16),
)

Set 16 bits of *r1* to the upper 16 bits of *imm19* (notated as *val*), starting from the quarter-word indexed by bits 1..2 of *imm19* (notated as *sh*), with bit 0 of *imm19* (notated as *c*) indicating whether to set the lower unmodified bits in *r1* to 0 and to sign-extend the upper unmodified bits in *r1*.

#code[```asm
ssi   r1, 0xFFFF, 0   ; c = 0; sh = 0
ssi   r1, 0xFFFF, 16  ; c = 0; sh = 1
ssi   r1, 0xFFFF, 32  ; c = 0; sh = 2
ssi   r1, 0xFFFF, 48  ; c = 0; sh = 3
ssi.c r1, 0xFFFF, 0   ; c = 1; sh = 0
ssi.c r1, 0xFFFF, 16  ; c = 1; sh = 1
ssi.c r1, 0xFFFF, 32  ; c = 1; sh = 2
ssi.c r1, 0xFFFF, 48  ; c = 1; sh = 3
```]
#code[```rust
let shift = sh << 4;
if c == 1 {
  gpr[r1] = (val as i64 << 48) >> (64 - shift);
} else {
  let mask = ~(0xFFFF << shift);
  gpr[r1] = (gpr[r1] & mask) | (val << shift);
}```]
#line(length: 100%)
]

#box[
=== ADD - Integer Add <ADD>

#fmt_c("imm9", "r3", "r2", "r1", ("000000"))

Add *r2* with the sum of *r3* and zero-extended *imm9* and store the result in *r1*.

#code[```asm
add r1, r2, r3      ; imm9 = 0
add r1, r2, r3, 511
```]
#code[```rust
gpr[r1] = gpr[r2] + (gpr[r3] + zero_extend(imm9));
```]
#line(length: 100%)
]

#box[
=== SUB - Integer Subtract <SUB>

#fmt_c("imm9", "r3", "r2", "r1", ("001000"))

Subtract *r3* from the sum of *r3* and zero-extended *imm9* and store the result in *r1*.

#code[```asm
sub r1, r2, r3      ; imm9 = 0
sub r1, r2, r3, 511
```]
#code[```rust
gpr[r1] = gpr[r2] - (gpr[r3] + zero_extend(imm9));
```]
#line(length: 100%)
]

#box[
=== MUL - Integer Multiply <MUL>

#fmt_c("imm9", "r3", "r2", "r1", ("010000"))

Multiply *r2* with the sum of *r3* and sign-extended *imm9* and place the lower 64 bits of the result into *r1*.

#code[```asm
mul r1, r2, r3      ; imm9 = 0
mul r1, r2, r3, 255
mul r1, r2, r3, -256
```]
#code[```rust
gpr[r1] = gpr[r2] * (gpr[r3] + sign_extend(imm9));
```]
#line(length: 100%)
]

#box[
=== UMULH - High Bits of Unsigned Integer Multiply <UMULH>

#fmt_c("imm9", "r3", "r2", "r1", ("110001"))

Multiply *r2* with the sum of *r3* and zero-extended *imm9* as 128-bit unsigned integers and place the upper 64 bits of the result into *r1*.

#code[```asm
umulh r1, r2, r3      ; imm9 = 0
umuhl r1, r2, r3, 511
```]
#code[```rust
gpr[r1] = ((gpr[r2] as u128 * 
          (gpr[r3] + zero_extend(imm9)) as u128) >> 64) as u64;
```]
#line(length: 100%)
]

#box[
=== IMULH - High Bits of Signed Integer Multiply <IMULH>

#fmt_c("imm9", "r3", "r2", "r1", ("111001"))

Multiply *r2* with the sum of *r3* and sign-extended *imm9* as 128-bit signed integers and place the upper 64 bits of the result into *r1*.

#code[```asm
imulh r1, r2, r3      ; imm9 = 0
imulh r1, r2, r3, 511
```]
#code[```rust
gpr[r1] = ((gpr[r2] as i128 * 
          (gpr[r3] + sign_extend(imm9)) as i128) >> 64) as u64;
```]
#line(length: 100%)
]

#box[
=== UDIV - Unsigned Integer Divide <UDIV>

#fmt_c("imm9", "r3", "r2", "r1", ("100000"))

Divide *r2* by the sum of *r3* and zero-extended *imm9* as unsigned integers and store the result in *r1*. If the divisor is 0, all bits of *r1* are set to 1.

#code[```asm
udiv r1, r2, r3      ; imm9 = 0
udiv r1, r2, r3, 511
```]
#code[```rust
let rhs = gpr[r3] + zero_extend(imm9);
gpr[r1] = if rhs != 0 {
    gpr[r2] as u64 / rhs as u64
} else {
    0xFFFFFFFFFFFFFFFF
};
```]
#line(length: 100%)
]

#box[
=== IDIV - Signed Integer Divide <IDIV>

#fmt_c("imm9", "r3", "r2", "r1", ("101000"))

Divide *r2* by the sum of *r3* and sign-extended *imm9* as signed integers and store the result in *r1*. If the divisor is 0, all bits of *r1* are set to 1.

#code[```asm
idiv r1, r2, r3      ; imm9 = 0
idiv r1, r2, r3, 255
idiv r1, r2, r3, -256
```]
#code[```rust
let rhs = gpr[r3] + sign_extend(imm9);
gpr[r1] = if rhs != 0 {
    gpr[r2] as i64 / rhs as i64
} else {
    0xFFFFFFFFFFFFFFFF
};
```]
#line(length: 100%)
]

#box[
=== UREM - Unsigned Integer Remainder <UREM>
#fmt_c("imm9", "r3", "r2", "r1", ("110000"))

Divide *r2* by *r3* as unsigned integers and store the remainder in *r1*. If the divisor is 0, all bits of *r1* are set to 1.

#code[```asm
urem r1, r2, r3      ; imm9 = 0
urem r1, r2, r3, 511
```]
#code[```rust
let rhs = gpr[r3] + zero_extend(imm9);
gpr[r1] = if rhs != 0 {
    gpr[r2] as u64 % rhs as u64
} else {
    0xFFFFFFFFFFFFFFFF
};
```]
#line(length: 100%)
]

#box[
=== IREM - Signed Integer Remainder <IREM>
#fmt_c("imm9", "r3", "r2", "r1", ("111000"))

Divide *r2* by *r3* as signed integers and store the remainder in *r1*. If the divisor is 0, all bits of *r1* are set to 1.

#code[```asm
irem r1, r2, r3      ; imm9 = 0
irem r1, r2, r3, 255
irem r1, r2, r3, -256
```]
#code[```rust
let rhs = gpr[r3] + sign_extend(imm9);
gpr[r1] = if rhs != 0 {
    gpr[r2] as i64 % lhs as i64
} else {
    0xFFFFFFFFFFFFFFFF
};
```]
#line(length: 100%)
]

#box[
=== ADDI - Integer Add Immediate <ADDI>
#fmt_b("imm14", "r2", "r1", ("000000"))

Add *r2* to zero-extended *imm14* and store the result in *r1*.

#code[```asm
addi r1, r2, 16383
```]
#code[```rust
gpr[r1] = gpr[r2] + zero_extend(imm14);
```]
#line(length: 100%)
]

#box[
=== SUBI - Integer Subtract Immediate <SUBI>
#fmt_b("imm14", "r2", "r1", ("001000"))

Subtract zero-extended *imm14* from *r2* and store the result in *r1*.

#code[```asm
subi r1, r2, 16383
```]
#code[```rust
gpr[r1] = gpr[r2] - zero_extend(imm14);
```]
#line(length: 100%)
]

#box[
=== MULI - Integer Multiply Immediate <MULI>
#fmt_b("imm14", "r2", "r1", ("010000"))

Multiply *r2* with sign-extended *imm14* and store the result in *r1*.

#code[```asm
muli r1, r2, 8191
muli r1, r2, -8192
```]
#code[```rust
gpr[r1] = gpr[r2] * sign_extend(imm14);
```]
#line(length: 100%)
]

#box[
=== UDIVI - Unsigned Integer Divide Immediate <UDIVI>
#fmt_b("imm14", "r2", "r1", ("100000"))

Divide *r2* as an unsigned integer by zero-extended *imm14* and store the result in *r1*. If the divisor is 0, all bits of *r1* are set to 1.

#code[```asm
udivi r1, r2, 16383
```]
#code[```rust
gpr[r1] = if imm14 != 0 {
    gpr[r2] as u64 / zero_extend(imm14)
} else {
    0xFFFFFFFFFFFFFFFF
};
```]
#line(length: 100%)
]

#box[
=== IDIVI - Signed Integer Divide Immediate <IDIVI>
#fmt_b("imm14", "r2", "r1", ("101000"))

Divide *r2* as a signed integer by sign-extended *imm14* and store the result in *r1*. If the divisor is 0, all bits of *r1* are set to 1.

#code[```asm
idivi r1, r2, 8191
idivi r1, r2, -8192
```]
#code[```rust
gpr[r1] = if imm14 != 0 {
    gpr[r2] as i64 / sign_extend(imm14)
} else {
    0xFFFFFFFFFFFFFFFF
};
```]
#line(length: 100%)
]

#box[
=== UREMI - Unsigned Integer Remainder Immediate <UREMI>
#fmt_b("imm14", "r2", "r1", ("110000"))

Divide *r2* as an unsigned integer by zero-extended *imm14* and store the remainder in *r1*. If the divisor is 0, all bits of *r1* are set to 1.

#code[```asm
uremi r1, r2, 16383
```]
#code[```rust
gpr[r1] = if imm14 != 0 {
    gpr[r2] as u64 % zero_extend(imm14)
} else {
    0xFFFFFFFFFFFFFFFF
};
```]
#line(length: 100%)
]

#box[
=== IREMI - Signed Integer Remainder Immediate <IREMI>
#fmt_b("imm14", "r2", "r1", ("111000"))

Divide *r2* as an signed integer by sign-extended *imm14* and store the remainder in *r1*. If the divisor is 0, all bits of *r1* are set to 1.

#code[```asm
iremi r1, r2, 8191
iremi r1, r2, -8192
```]
#code[```rust
gpr[r1] = if imm14 != 0 {
    gpr[r2] as i64 % sign_extend(imm14)
} else {
    0xFFFFFFFFFFFFFFFF
};
```]
#line(length: 100%)
]

== Bitwise Logic

#box[
=== AND - Logical And <AND>
#fmt_c("imm9", "r3", "r2", "r1", ("000001"))

'AND' *r2* with the 'OR' of *r3* and zero-extended *imm9* and store the result in *r1*.

#code[```asm
and r1, r2, r3      ; imm9 = 0
and r1, r2, r3, 511
```]
#code[```rust
gpr[r1] = gpr[r2] & (gpr[r3] | zero_extend(imm9));
```]
#line(length: 100%)
]

#box[
=== OR  - Logical Or <OR>
#fmt_c("imm9", "r3", "r2", "r1", ("001001"))

'OR' *r2* with the 'OR' of *r3* and zero-extended *imm9* and store the result in *r1*.

#code[```asm
or r1, r2, r3      ; imm9 = 0
or r1, r2, r3, 511
```]
#code[```rust
gpr[r1] = gpr[r2] | (gpr[r3] | zero_extend(imm9));
```]
]

#box[
=== NOR  - Logical Nor <NOR>
#fmt_c("imm9", "r3", "r2", "r1", ("010001"))

'NOR' *r2* with the 'OR' of *r3* and zero-extended *imm9* and store the result in *r1*.

#code[```asm
nor r1, r2, r3      ; imm9 = 0
nor r1, r2, r3, 511
```]
#code[```rust
gpr[r1] = ~(gpr[r2] | (gpr[r3] | zero_extend(imm9)));
```]
#line(length: 100%)
]

#box[
=== XOR - Logical Exclusive Or <XOR>
#fmt_c("imm9", "r3", "r2", "r1", ("011001"))

'XOR' *r2* with the 'OR' of *r3* and zero-extended *imm9* and store the result in *r1*.

#code[```asm
xor r1, r2, r3      ; imm9 = 0
xor r1, r2, r3, 511
```]
#code[```rust
gpr[r1] = gpr[r2] ^ (gpr[r3] | zero_extend(imm9));
```]
]

#box[
=== ANDI - Logical And Immediate <ANDI>
#fmt_b("imm14", "r2", "r1", ("000001"))

'AND' *r2* with zero-extended *imm14* and store the result in *r1*.

#code[```asm
andi r1, r2, 16383
```]
#code[```rust
gpr[r1] = gpr[r2] & zero_extend(imm14);
```]
#line(length: 100%)
]

#box[
=== ORI  - Logical Or Immediate <ORI>
#fmt_b("imm14", "r2", "r1", ("001001"))

'OR' *r2* with zero-extended *imm14* and store the result in *r1*.

#code[```asm
ori r1, r2, 16383
```]
#code[```rust
gpr[r1] = gpr[r2] | zero_extend(imm14);
```]
#line(length: 100%)
]

#box[
=== NORI  - Logical Nor Immediate <NORI>
#fmt_b("imm14", "r2", "r1", ("010001"))

'NOR' *r2* with zero-extended *imm14* and store the result in *r1*.

#code[```asm
nori r1, r2, 16383
```]
#code[```rust
gpr[r1] = ~(gpr[r2] | zero_extend(imm14));
```]
#line(length: 100%)
]

#box[
=== XORI - Logical Exclusive Or Immediate <XORI>
#fmt_b("imm14", "r2", "r1", ("011001"))

'XOR' *r2* with zero-extended *imm14* and store the result in *r1*.

#code[```asm
xori r1, r2, 16383
```]
#code[```rust
gpr[r1] = gpr[r2] ^ zero_extend(imm14);
```]
#line(length: 100%)
]

#box[
=== SL  - Shift Left <SL>
#fmt_c("imm9", "r3", "r2", "r1", ("100010"))

Shift *r2* left by the sum (modulo 64) of zero-extended *imm9* and *r3*, and store the result in *r1*.

#code[```asm
sl r1, r2, r3, 63
sl r1, r2, 63     ; r3 = zr
sl r1, r2, r3     ; imm9 = 0
```]
#code[```rust
let shamt = (gpr[r3] + zero_extend(imm9)) & 0b111111;
gpr[r1] = gpr[r2] << shamt;
```]
#line(length: 100%)
]

#box[
=== USR - Unsigned Shift Right <USR>
#fmt_c("imm9", "r3", "r2", "r1", ("010010"))

Shift *r2* right by the sum (modulo 64) of zero-extended *imm9* and *r3*, and store the result in *r1*. "Empty" bits at the most-significant end are set to zero.

#code[```asm
usr r1, r2, r3, 63
usr r1, r2, 63     ; r3 = zr
usr r1, r2, r3     ; imm9 = 0
```]
#code[```rust
let shamt = (gpr[r3] + zero_extend(imm9)) & 0b111111;
gpr[r1] = gpr[r2] as u64 >> shamt;
```]
#line(length: 100%)
]

#box[
=== ISR - Signed Shift Right <ISR>
#fmt_c("imm9", "r3", "r2", "r1", ("011010"))

Shift *r2* right by the sum (modulo 64) of zero-extended *imm9* and *r3*, and store the result in *r1*. "Empty" bits at the most-significant end are copied from bit 63 of *r2*.

#code[```asm
isr r1, r2, r3, 63
isr r1, r2, 63     ; r3 = zr
isr r1, r2, r3     ; imm9 = 0
```]
#code[```rust
let shamt = (gpr[r3] + zero_extend(imm9)) & 0b111111;
gpr[r1] = gpr[r2] as i64 >> shamt;
```]
#line(length: 100%)
]

#box[
=== SI - Shift Immediate <SI>
// #fmt_b("imm14", "r2", "r1", ("000010"))
#format(
  ("1", 1),
  ("0", 1),
  ("000010", 6),
  ("r1", 5),
  ("r2", 5),
  ("lsh", 6),
  ("rsh", 6),
  ("i", 1),
  ("unused(hidden)", 1),
)

Shift *r2* left by bits 0..5 of *imm14* (notated as *lsh*), then shift right by bits 6..11 of *imm14* (notated as *rsh*). If bit 12 of *imm14* (notated as *i*) is set, the right shift is signed, otherwise unsigned.

#code[```asm
si.u r1, r2, 63, 63 ; lsh = 1st, rhs = 2nd, i = 0
si.i r1, r2, 63, 63 ; lsh = 1st, rhs = 2nd, i = 1
```]
#code[```rust
gpr[r1] = if i == 0 {
  (gpr[r2] << lsh) as u64 >> rsh
} else {
  (gpr[r2] << lsh) as i64 >> rsh
};
```]
#line(length: 100%)
]

#box[
=== CB - Clear Bits <CB>
// #fmt_b("imm14", "r2", "r1", ("001010"))
#format(
  ("1", 1),
  ("0", 1),
  ("001010", 6),
  ("r1", 5),
  ("r2", 5),
  ("lsh", 6),
  ("rsh", 6),
  ("unused(hidden)", 2),
)

Shift an all-bits-set mask by left by bits 0..5 of *imm14* (notated as *lsh*), then shift right (unsigned) by bits 6..11 of *imm14* (notated as *rsh*). Clear bits in *r2* corresponding to set bits of the mask and store the result in *r1*.

#code[```asm
cb r1, r2, 63, 63 ; lsh = 1st, rhs = 2nd
```]
#code[```rust
gpr[r1] = gpr[r2] & 
  ~((0xFFFFFFFFFFFFFFFF as u64 << lsh) as u64 >> rhs);
```]
#line(length: 100%)
]

#box[
=== ROR - Rotate Right <ROR>
#fmt_c("imm9", "r3", "r2", "r1", ("000010"))

Rotate *r2* right according to the sum of zero-extended *imm9* and *r3* as an unsigned integer and store the result in *r1*.

#code[```asm
ror r1, r2, r3, 63
ror r1, r2, 63     ; r3 = zr
ror r1, r2, r3     ; imm9 = 0
```]
#code[```rust
let shamt = (gpr[r3] + zero_extend(imm9)) & 0b111111;
gpr[r1] = (gpr[r2] as u64 >> shamt) 
        | (gpr[r2] as u64 << (64 - shamt));
```]
#line(length: 100%)
]

#box[
=== ROL - Rotate Left <ROL>
#fmt_c("imm9", "r3", "r2", "r1", ("001010"))

Rotate *r2* left according to the sum of zero-extended *imm9* and *r3* as an unsigned integer and store the result in *r1*.

#code[```asm
rol r1, r2, r3, 63
rol r1, r2, 63     ; r3 = zr
rol r1, r2, r3     ; imm9 = 0
```]
#code[```rust
let shamt = (gpr[r3] + zero_extend(imm9)) & 0b111111;
gpr[r1] = (gpr[r2] as u64 << shamt) 
        | (gpr[r2] as u64 >> (64 - shamt));
```]
#line(length: 100%)
]

#box[
=== REV - Reverse Bits <REV>
// #fmt_b("imm19", "r2", "r1", ("010010"))
#format(
  ("1", 1),
  ("0", 1),
  ("010010", 6),
  ("r1", 5),
  ("r2", 5),
  ("set", 6),
  ("unused", 8),
)

Reverse bits of *r2* based on bits 0..5 of *imm19* (notated as *set*) and store the result to *r1* and store the result in *r1*.

#code[```asm
rev     r1, l1, 0b111111
rev.h   r1, r2 ; set = 0b100000
rev.q   r1, r2 ; set = 0b110000
rev.b   r1, r2 ; set = 0b111000
rev.bit r1, r2 ; set = 0b111111
```]
#code[```rust
let mut value = gpr[r2];
if set & 0b100000 != 0 {
  value = (0xFFFFFFFF00000000 & value) >> 32
        | (0x00000000FFFFFFFF & value) << 32;
}
if set & 0b10000 != 0 {
  value = (0xFFFF0000FFFF0000 & value) >> 16
        | (0x0000FFFF0000FFFF & value) << 16;
}
if set & 0b1000 != 0 {
  value = (0xFF00FF00FF00FF00 & value) >> 8
        | (0x00FF00FF00FF00FF & value) << 8;
}
if set & 0b100 != 0 {
  value = (0xF0F0F0F0F0F0F0F0 & value) >> 4
        | (0x0F0F0F0F0F0F0F0F & value) << 4;
}
if set & 0b10 != 0 {
  value = (0xCCCCCCCCCCCCCCCC & value) >> 2
        | (0x3333333333333333 & value) << 2;
}
if set & 0b1 != 0  {
  value = (0xAAAAAAAAAAAAAAAA & value) >> 1
        | (0x5555555555555555 & value) << 1;
}
gpr[r1] = value;
```]
#line(length: 100%)
]

#box[
=== CSB - Count Set Bits <CSB>
#fmt_b("unused", "r2", "r1", ("110001"))

Count the number of set bits in *r2* and store the count in *r1*.

#code[```asm
csb r1, r2
```]
#code[```rust
let mut count = 0;
for i in 0..64 {
  if (1 << i) & gpr[r2] != 0 {
    count += 1;
  }
}
gpr[r1] = count;
```]
#line(length: 100%)
]

#box[
=== CLZ - Count Leading Zeroes <CLZ>
#fmt_b("unused", "r2", "r1", ("100001"))

Count the number of leading (most-significant) bits in *r2* that are set to 0 and store the count in *r1*.

#code[```asm
clz r1, r2
```]
#code[```rust
let mut count = 0;
// 63, 62, 61, etc.
for i in (0..64).rev() {
  if (1 << i) & gpr[r2] != 0 {
    break;
  }
  count += 1;
}
gpr[r1] = count;
```]
#line(length: 100%)
]

#box[
=== CTZ - Count Trailing Zeroes <CTZ>
#fmt_b("unused", "r2", "r1", ("101001"))

Count the number of trailing (least-significant) bits in *r2* that are set to 0 and store the count in *r1*.

#code[```asm
ctz r1, r2
```]
#code[```rust
let mut count = 0;
for i in 0..64 {
  if (1 << i) & gpr[r2] != 0 {
    break;
  }
  count += 1;
}
gpr[r1] = count;
```]
#line(length: 100%)
]

#box[
=== EXT - Extract Bits <EXT>
#fmt_c("unused", "r3", "r2", "r1", ("100001"))

Extract bits from *r2* according to a mask *r3* and place masked bits contiguously into *r1*. The inverse operation to `dep`.

#code[```asm
ext r1, r2, r3
```]
#code[```rust
let mut result = 0;
let mut k = 0;
for i in 0..64 {
  // if mask bit is set
  if (1 << i) & gpr[r3] != 0 {
    // get corresponding bit from source
    let bit = (gpr[r2] >> i) & 1;
    // place source bit at next location
    result |= bit << k;
    k += 1;
  }
}
gpr[r1] = result;
```]
#line(length: 100%)
]

#box[
=== DEP - Deposit Bits <DEP>
#fmt_c("unused", "r3", "r2", "r1", ("101001"))

Deposit contiguous bits from *r2* according to a mask *r3* and place masked bits non-contiguously into *r1*. The inverse operation to `ext`.

#code[```asm
dep r1, r2, r3
```]
#code[```rust
let mut result = 0;
let mut k = 0;
for i in 0..64 {
  // if mask bit is set
  if (1 << i) & gpr[r3] != 0 {
    // get next bit from source
    let bit = (gpr[r2] >> k) & 1;
    // place source bit at mask bit location
    result |= bit << i;
    k += 1;
  }
}
gpr[r1] = result;
```]
#line(length: 100%)
]


== Comparison

#box[
=== SEQ - Set Equal <SEQ>
#fmt_c("imm9", "r3", "r2", "r1", ("100011"))

if *r2* is equal to the sum of sign-extended *imm9* and *r3*, set *r1* to 1, otherwise set *r1* to 0.

#code[```asm
seq r1, r2, r3      ; imm9 = 0
seq r1, r2, r3, 511
```]
#code[```rust
gpr[r1] = (gpr[r2] == gpr[r3] + sign_extend(imm9)) as u64;
```]
#line(length: 100%)
]

#box[
=== SULT - Set Unsigned Less Than <SULT>
#fmt_c("imm9", "r3", "r2", "r1", ("000011"))

if *r2* is less than the sum of zero-extended *imm9* and *r3* as unsigned integers, set *r1* to 1, otherwise set *r1* to 0.

#code[```asm
sult r1, r2, r3      ; imm9 = 0
sult r1, r2, r3, 511
```]
#code[```rust
gpr[r1] = (gpr[r2] as u64 < (gpr[r3] + zero_extend(imm9)) as u64) as u64;
```]
#line(length: 100%)
]

#box[
=== SILT - Set Signed Less Than <SILT>
#fmt_c("imm9", "r3", "r2", "r1", ("001011"))

if *r2* is less than the sum of sign-extended *imm9* and *r3* as signed integers, set *r1* to 1, otherwise set *r1* to 0.

#code[```asm
silt r1, r2, r3       ; imm9 = 0
sile r1, r2, r3, 255
sile r1, r2, r3, -256
```]
#code[```rust
gpr[r1] = (gpr[r2] as i64 < (gpr[r3] + zero_extend(imm9)) as i64) as u64;
```]
#line(length: 100%)
]

#box[
=== SULE - Set Unsigned Less or Equal <SULE>
#fmt_c("imm9", "r3", "r2", "r1", ("010011"))

if *r2* is less than or equal to the sum of zero-extended *imm9* and *r3* as unsigned integers, set *r1* to 1, otherwise set *r1* to 0.

#code[```asm
sule r1, r2, r3
sule r1, r2, r3, 511
```]
#code[```rust
gpr[r1] = (gpr[r2] as u64 < (gpr[r3] + zero_extend(imm9)) as u64) as u64;
```]
#line(length: 100%)
]

#box[
=== SILE - Set Signed Less or Equal <SILE>
#fmt_c("imm9", "r3", "r2", "r1", ("011011"))

if *r2* is less than or equal to the sum of sign-extended *imm9* and *r3* as signed integers, set *r1* to 1, otherwise set *r1* to 0.

#code[```asm
sile r1, r2, r3
sile r1, r2, r3, 255
sile r1, r2, r3, -255
```]
#code[```rust
gpr[r1] = (gpr[r2] as i64 <= (gpr[r3] + sign_extend(imm9)) as i64) as u64;
```]
#line(length: 100%)
]

#box[
=== SEQI - Set Equal Immediate <SEQI>
#fmt_b("imm14", "r2", "r1", ("100011"))

if *r2* is equal to sign-extended *imm14*, set *r1* to 1, otherwise set *r1* to 0.

#code[```asm
seqi r1, r2, 8191
seqi r1, r2, -8192
```]
#code[```rust
gpr[r1] = (gpr[r2] as i64 == sign_extend(imm14)) as u64;
```]
#line(length: 100%)
]

#box[
=== SULTI - Set Unsigned Less Than Immediate <SULTI>
#fmt_b("imm14", "r2", "r1", ("000011"))

if *r2* as an unsigned integer is less than zero-extended *imm14*, set *r1* to 1, otherwise set *r1* to 0.

#code[```asm
sulti r1, r2, 16383
```]
#code[```rust
gpr[r1] = (gpr[r2] as u64 < zero_extend(imm14)) as u64;
```]
#line(length: 100%)
]

#box[
=== SILTI - Set Signed Less Than Immediate <SILTI>
#fmt_b("imm14", "r2", "r1", ("001011"))

if *r2* as a signed integer is less than or equal to sign-extended *imm14*, set *r1* to 1, otherwise set *r1* to 0.

#code[```asm
silti r1, r2, 8191
silti r1, r2, -8192
```]
#code[```rust
gpr[r1] = (gpr[r2] as i64 < sign_extend(imm14)) as u64;
```]
#line(length: 100%)
]

#box[
=== SULEI - Set Unsigned Less or Equal Immediate <SULEI>
#fmt_b("imm14", "r2", "r1", ("010011"))

if *r2* as an unsigned integer is less than or equal to zero-extended *imm14*, set *r1* to 1, otherwise set *r1* to 0.

#code[```asm
sulei r1, r2, 16383
```]
#code[```rust
gpr[r1] = (gpr[r2] as u64 <= zero_extend(imm14)) as u64;
```]
#line(length: 100%)
]

#box[
=== SILEI - Set Signed Less or Equal Immediate <SILEI>
#fmt_b("imm14", "r2", "r1", ("011011"))

if *r2* as a signed integer is less than or equal to sign-extended *imm14*, set *r1* to 1, otherwise set *r1* to 0.

#code[```asm
silei r1, r2, 8191
silei r1, r2, -8192
```]
#code[```rust
gpr[r1] = (gpr[r2] as i64 <= sign_extend(imm14)) as u64;
```]
#line(length: 100%)
]

== Control Flow

#box[
=== BZ - Branch If Zero <BZ>
#fmt_a("imm19", "r1", ("110100"))

if *r1* is equal to 0, set `ip` to the sum of sign-extended *imm19* shifted left by 2 and `ip`.

#code[```asm
bz r1, symbol ; imm19 = (symbol - addr - 4) >> 2
```]
#code[```rust
if gpr[r1] == 0 {
  gpr[IP] = gpr[IP] + sign_extend(imm19) << 2;
}
```]
#line(length: 100%)
]

#box[
=== BN - Branch If Not Zero <BN>
#fmt_a("imm19", "r1", ("111100"))

if *r1* is not equal to 0, set `ip` to the sum of sign-extended *imm19* shifted left by 2 and `ip`.

#code[```asm
bn r1, symbol ; imm19 = (symbol - addr - 4) >> 2
```]
#code[```rust
if gpr[r1] != 0 {
  gpr[IP] = gpr[IP] + sign_extend(imm19) << 2;
}
```]
#line(length: 100%)
]

#box[
=== JL - Jump and Link <JL>
#fmt_b("imm14", "r2", "r1", ("101100"))

Set *r1* to `ip` and set `ip` to the sum of zero-extended *imm14* shifted left by 2 and *r2*.

#code[```asm
jl r1, r2, 8191
jl r1, r2, -8192
jl r1, r2        ; imm14 = 0
```]
#code[```rust
let ip = gpr[IP];
gpr[IP] = gpr[r2] + zero_extend(imm14) << 2;
gpr[r1] = ip;
```]
#line(length: 100%)
]

#box[
=== JLR - Jump and Link Relative <JLR>
#fmt_b("imm14", "r2", "r1", ("100100"))

Set *r1* to `ip` and set `ip` to the sum of `ip`, zero-extended *imm14* shifted left by 2, and *r2*.

#code[```asm
jlr r1, r2, 8191
jlr r1, r2, -8192
jlr r1, r2        ; imm14 = 0
```]
#code[```rust
let ip = gpr[IP];
gpr[IP] += gpr[r2] + zero_extend(imm14) << 2;
gpr[r1] = ip;
```]
#line(length: 100%)
]

== System Control

#box[
=== SYSCALL - System Call Interrupt <SYSCALL>
#fmt_a("unused", "unused", ("000111"))
Trigger a `SYSCALL` interrupt.

#code[```asm
syscall
```]
#code[```rust
trigger_interrupt(SYSCALL);
```]
#line(length: 100%)
]

#box[
=== BREAKPT - Breakpoint Interrupt <BREAKPT>
#fmt_a("unused", "unused", ("001111"))
Trigger a `BREAKPT` interrupt.

#code[```asm
breakpt
```]
#code[```rust
trigger_interrupt(BREAKPT);
```]
#line(length: 100%)
]

#box[
=== WAIT - Wait for Interrupt <WAIT>
#fmt_a("unused", "unused", ("111111"))
Pause execution and idle until an interrupt occurs. Must be in kernel mode to execute.

#code[```asm
wait
```]
#code[```rust
if ctrl[STAT] & STAT_U != 0 {
  trigger_interrupt(INVALID);
}
pause_execution();
```]
#line(length: 100%)
]

#box[
=== SPIN - Hint Spin-wait Loop <SPIN>
#fmt_a("unused", "unused", ("010111"))
Hint a spin-wait loop to this LP. On complex implementations, this instruction may lower power consumption in spin-wait loops, mitigate performance issues due to execution pipelining, signal that this LP is currently not busy for hyper-threading mechanisms, etc.

#code[```asm
spin
```]
#code[```rust
hint_spin();
```]
#line(length: 100%)
]

#box[
=== IRET - Interrupt Return <IRET>
#fmt_a("unused", "unused", ("100111"))
Return from an interrupt handler or jump into user mode. Must be in kernel mode to execute.

#code[```asm
iret
```]
#code[```rust
if ctrl[STAT] & STAT_U != 0 {
  trigger_interrupt(INVALID);
}
ctrl[STAT] = ctrl[INTSTAT];
gpr[IP] = ctrl[INTIP];
```]
#line(length: 100%)
]

#box[
=== LCTRL - Load Control Register <LCTRL>
#fmt_a("imm19", "r1", ("101111"))
Load from control register *imm19* to *r1*. Must be in kernel mode to execute.

#code[```asm
lctrl r1, int0
```]
#code[```rust
if ctrl[STAT] & STAT_U != 0 || !is_valid_ctrl(imm19) {
  trigger_interrupt(INVALID);
}
gpr[r1] = ctrl[imm19];
```]
#line(length: 100%)
]

#box[
=== SCTRL - Store Control Register <SCTRL>
#fmt_a("imm19", "r1", ("110111"))
Store *r1* to control register *imm19*. Must be in kernel mode to execute.

#code[```asm
sctrl int0, r1
```]
#code[```rust
if ctrl[STAT] & STAT_U != 0 || !is_valid_ctrl(imm19) {
  trigger_interrupt(INVALID);
}
ctrl[imm19] = gpr[r1];
```]
#line(length: 100%)
]









/*
    call - chooses between scall and rcall based on context

    // short call, for intra-section calling
    scall r1, symbol
        // if the symbol is defined in the same text section (and is less than +-32764 bytes away):
        jlr r1, zr, offsetof symbol >> 2
        // `scall` does not produce relocations.

    // relative call, good enough for 99% of calls
    rcall r1, r2, symbol
        // can reach anything -2 to 2GB relative
        ssi.c r2, offsetof symbol >> 16 & 0xFFFF, 16
        jlr r1, r2, (offsetof symbol & 0xFFFF) >> 2

    // alternative call syntax, uses the symbol register as the link register
    rcall r1, symbol
        call r1, r1, symbol

    // far call, loads the entire symbol in as an absolute value
    fcall r1, r2, symbol
        ssi.c r2, symbol >> 48 & 0xFFFF, 48
        ssi r2, symbol >> 32 & 0xFFFF, 32
        ssi r2, symbol >> 16 & 0xFFFF, 16
        jl r1, r2, (symbol & 0xFFFF) >> 2

    // alternative call syntax, uses the symbol register as the link register
    fcall r1, symbol
        fcall r1, r1, symbol

    ret
        jl zr, lr, 0

    nop
        or zr, zr, zr

    mov r1, r2
        or r1, r2, zr

*/

#pagebreak()

== Pseudo-instructions

Assemblers for the Aphelion ISA must implement the following pseudo-instructions to be compliant.

Some instructions may produce *linker relocations* so that symbol values that change during linking are accurately updated where they are referenced. Relocation types are detailed in the Aphelion ABI Specification.

Note that the value of *addr* in the following expansions is a shorthand for the address of the _first_ instruction in the expansion.

// #box[
// === SCALL - Short Procedure Call
// Perform a relative, short-distance procedure call to *symbol* and save the return address to *r1*. This may only reference symbols from the same section as the expansion, as this pseudo-instruction does not produce a relocation.

// #code[```asm
// scall r1, symbol
// scall symbol     ; r1 = lp
//   ; Expands to:
// jlr r1, zr, (symbol - addr - 4) >> 2
// ```]
// #line(length: 100%)
// ]

// #box[
// === RCALL - Relative Procedure Call
// Perform a relative procedure call to *symbol* and save the return address to *r1*, using *r2* as a scratch register. Produces an `RCALL` relocation at the address of the `ssi.c` instruction.

// #code[```asm
// rcall r1, r2, symbol
// rcall r1, symbol     ; r2 = r1
// rcall symbol         ; r1 = lp, r2 = lp
//   ; Expands to:
// ssi.c r2, ((symbol - addr) >> 16) & 0xFFFF, 16
// jlr   r1, r2, ((symbol - addr) & 0xFFFF) >> 2
// ```]
// #line(length: 100%)
// ]

#box[
=== CALL - Procedure Call
Perform a relative procedure call to *symbol* and save the return address to *r1*, using *r2* as a scratch register. Produces a `CALL` relocation at the address of the `ssi.c` instruction.

#code[```asm
call r1, r2, symbol
call r1, symbol     ; r2 = r1
call symbol         ; r1 = lp, r2 = lp
  ; Expands to:
ssi.c r2, ((symbol - addr) >> 16) & 0xFFFF, 16
jlr   r1, r2, ((symbol - addr) & 0xFFFF) >> 2
```]

Optionally, if *symbol* is a symbol or a symbol offset that _does not_ require a relocation (within the same section, between pinned sections, etc.), assemblers may produce a smaller, logically equivalent expansion if possible.

#line(length: 100%)
]

#box[
=== FCALL - Far Procedure Call
Perform an absolute procedure call to *symbol* and save the return address to *r1*, using *r2* as a scratch register. Produces an `FCALL` relocation at the address of the `ssi.c` instruction.

#code[```asm
fcall r1, r2, symbol
fcall r1, symbol     ; r2 = r1
fcall symbol         ; r1 = lp, r2 = lp
  ; Expands to:
ssi.c r2, (symbol >> 48) & 0xFFFF, 48
ssi   r2, (symbol >> 32) & 0xFFFF, 32
ssi   r2, (symbol >> 16) & 0xFFFF, 16
jl    r1, r2, (symbol & 0xFFFF) >> 2
```]
#line(length: 100%)
]

#box[
=== LI - Load Immediate
Load a constant, symbol, or symbol offset (notated as *val*) into *r1*.

If *val* is a constant, a sequence of `ssi` instructions shall be used. The assembler should aim to minimize the size of the sequence, though simple implementations may choose to always use the full four-instruction expansion, shown below:

#code[```asm
li r1, val
  ; Expands to:
ssi.c r1, val >> 48, 48
ssi   r1, (val >> 32) & 0xFFFF, 32
ssi   r1, (val >> 16) & 0xFFFF, 16
ssi   r1, val & 0xFFFF, 0
```]

If *val* is a symbol or a symbol offset, the above expansion shall be used and an `LI` relocation shall be produced at the address of the `ssi.c` instruction.

Optionally, if *val* is a symbol or a symbol offset that _does not_ require a relocation (within the same section, between pinned sections, etc.), arithmetic involving the instruction pointer may be used to produce a smaller expansion.

#line(length: 100%)
]

#box[
=== RET - Return from Procedure Call
Return from a standard procedure call by performing a jump to the address in `r1`.

#code[```asm
ret r1
ret    ; r1 = lp
  ; Expands to:
jl zr, r1, 0
```]
#line(length: 100%)
]


#box[
=== NOP - No Operation
The canonical way to do nothing.

#code[```asm
nop
  ; Expands to:
or zr, zr, zr
```]
#line(length: 100%)
]

#box[
=== MOV - Move Register
Copy the value of `r2` into `r1`.

#code[```asm
mov r1, r2
  ; Expands to:
or r1, r2, zr
```]
#line(length: 100%)
]

#pagebreak()
= Glossary

- *word*: The size of a register. 8 bytes, 64 bits.
- *half-word*: 4 bytes, 32 bits.
- *quarter-word*: 2 bytes, 16 bits.
- *logical processor*, *LP*: a distinct Aphelion context that executes instructions independently from other contexts in a system.
- *page*: a contiguous block of memory 4KiB ($2^12$B) in size.
- *weak consistency*: a memory model which allows global reordering of memory accesses, restricted by memory fence operations.
- *data cache*, *d-cache*: internal memory used to speed up memory loads/stores.
- *instruction cache*, *i-cache*: internal memory used to speed up instruction fetches and aid the execution pipeline.
- *(linker) relocation*: An object file entry that inserts/replaces a value in a section such that symbol values that change during linking are accurately updated where they are referenced.
