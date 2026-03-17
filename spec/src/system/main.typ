#import "@preview/cetz:0.4.2"

#let title = "Aphelion System Environment Specification"
#let subtitle = "Version 6"
#let authors = (
  (name: "Seth Poulsen", email: "sp@sandwichman.dev"),
)

#set raw(syntaxes: "../../assets/aphel.sublime-syntax");
#set raw(syntaxes: "../../assets/mars.sublime-syntax");
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
    _*#{title}* #{subtitle}_
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

This document specifies the system environment, memory layout, and firmware services on standard Aphelion systems.

== Formatting

All code snippets in this document are in the Mars programming language. It should be relatively easy to use and read for those familiar with systems languages such as C. Along with their high-level Mars representations, all function signatures will be described directly in Aphelion assembly for ease of reference.

= Physical Address Space

Access failures on the physical address space shall generate a `BUS*` interrupt corresponding to the mode of access. Access failures will also occur due to any access deemed invalid by the bus recipient that handles it. This may be due to completely unmapped portions of memory, unsupported access types/modes for certain MMIO devices, etc.

When a memory access is made, the recipient of the access can see:
- The access address itself;
- The access mode (read, write, or execute);
- The ID of the LP that issued it;
- The LP's mode (kernel or user);

The recipient may accept or reject accesses based on any of these factors.

== RAM

There is a maximum of 512 RAM slots, each with a maximum size of 256 GiB. The first slot is mapped to `0x00000000_00000000` and further slots are mapped to consecutive multiples of 256 GiB. Thus, the maximum possible RAM-mapped address is `0x00007FFF_FFFFFFFF`.

Each active slot must provides a contiguous region of memory starting at its slot address, accessible by all access widths, all access types, and to all LPs. Memory provided in each slot must be in units of whole pages (4KiB). The amount of memory provided by each slot is given by the System Description Table.

This region of memory may always be cached.

== Memory-Mapped Devices

Direct device interfaces may be mapped by implementation hardware and firmware in the range of `0x00008000_00000000` to `0xFFFFFFFE_FFFFFFFF`.

This region of memory may not be cached.

== System-Reserved Region

The *System-Reserved Region* (SRR) is a region of the address space from `0xFFFFFFFF_00000000` to `0xFFFFFFFF_FFFFFFFF` (4GiB), where the firmware and relevant implementation-specific details, control registers, and firmware reside. 

Accesses to addresses inside this region may be statically or dynamically restricted through implementation-specific means.

Portions of this region may or may not be cached, depending on the implementation.

#pagebreak()
= Firmware Services

#code[```mars
struct ServiceTable {
    sdt_get: ^fun(): ^const SystemDescriptionTable,
    current_lp_get: ^fun(): ^const LpInfo,
    
    serial_put: ^fun(c: u32),
    serial_get: ^fun(): ?u32,

    power_state_set: ^fun(state: PowerState),

    env_set:    ^fun(name, value: []const u8, perms: u8),
    env_get:    ^fun(name: []const u8): ?[]const u8,
    env_delete: ^fun(name: []const u8),

    update: ^fun(bin: []const u8): ?[]const u8,
}
```]

#code[```mars
struct EnvPerms {
    // visible to other hosts?
    def GLOBAL_R: u8 = 1 << 0;
    // writeable by other hosts?
    def GLOBAL_W: u8 = 1 << 1;
}
```]

#code[```mars
struct SystemDescriptionTable {
    lps: ^[512]LpInfo,
}
```]

#code[```mars
struct LpInfo {
    clock_speed: u64,

    // on NUMA systems, this is a list of indices to 
    // active RAM slots, sorted by fastest-to-slowest 
    // access time on this LP. otherwise, null.
    preferred_memory: ?[]u8
}
```]

#code[```mars
enum PowerState {
    Shutdown = 0,
    Restart  = 1,
}
```]

// == `sdt_get` - Locate System Description Table

#pagebreak()
= System Description Table

= Devices
#code[```mars
struct DeviceInfo {
    address: [^]u8,

    vendor:  [^:'0']u8,
    product: [^:0]u8,
}
```]

#pagebreak()
= Booting Host Software

#pagebreak()
= Interrupt Architecture

#pagebreak()
= Glossary

- *host* / *host software*: The program loaded in and booted from disk by the firmware.
- *word*: The size of a register. 8 bytes, 64 bits.
- *half-word*: 4 bytes, 32 bits.
- *quarter-word*: 2 bytes, 16 bits.
