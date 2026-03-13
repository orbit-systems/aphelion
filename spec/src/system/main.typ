#import "@preview/cetz:0.4.2"

#let title = "Aphelion System Environment Specification"
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

This document specifies the environment, interface, and firmware services on standard Aphelion systems.

= Physical Address Space Layout

// #align(center, table(
//   align: center + horizon,
//   rows: (1cm, 2cm, 1cm),
//   [RAM Slot 0],
//   [ . . . ],
//   [RAM Slot 511],
// ))

== RAM

There are 512 mappable ram slots, each with a maximum size of 256 GiB. The first slot is mapped to `0x00000000_00000000` and further slots are mapped to consecutive multiples of 256 GiB. Thus, the maximum possible RAM-mapped address is `0x000007FFF_FFFFFFFF`.

Each active slot provides a contiguous region of 4KiB pages, accessible by all access widths, all access types, and to all cores.

The amount of memory provided by each slot is given by the System Description Table.

== Memory-Mapped Devices

== System-Reserved Region

The *System-Reserved Region* (SRR) is a region of the address space from `0xFFFFFFFF_00000000` to `0xFFFFFFFF_FFFFFFFF` (4GiB), where the firmware and relevant implementation-specific details, control registers, and firmware reside. 

Accesses to addresses inside this region may be statically or dynamically restricted through implementation-specific means. Restricted accesses will trigger the relevant bus fault interrupt.


#pagebreak()
= System Description Table

#pagebreak()
= Glossary

- *word*: The size of a register. 8 bytes, 64 bits.
- *half-word*: 4 bytes, 32 bits.
- *quarter-word*: 2 bytes, 16 bits.
