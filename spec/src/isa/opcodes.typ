
#let orbit_grey = rgb("f2f2f2")
#let orbit_red  = rgb("f80530")
#let grey_cell = table.cell(fill: orbit_grey)[]

#let ccenter(a) = table.cell(a, align: center)

== Opcode Mapping
Each instruction is assigned a format-specific 6-bit opcode. This 6-bit opcode is
comprised of a major opcode in bits 0..2 and a minor opcode in bits 3..5. The major
opcode roughly defines a “family” of behavior.

Instructions with similar behavior are designed to be similar in encoding. For
example, arithmetic operations with both a register form and immediate form have
identical 6-bit opcodes.

Another useful example of this is with memory loads and stores in Format C, where
lower two bits of the minor opcode specify the size of the access, the high bit of the
minor opcode specifies LL/SC behavior, and the major opcode distinguishes between
load and store operations.

#let op(str) = link(label(str), str)

=== Format A

#table(
  columns: (auto, 1fr, 1.1fr, 1fr, 1fr, 1.1fr, 1fr, 1fr, 1.2fr),
  fill: (x, y) => if y == 0 or y == 1 or x == 0 {orbit_grey},
  table.cell("Minor", rowspan: 2, align: horizon + center), 
  table.cell("Major", colspan: 8, align: center),
  table.hline(stroke: 0pt),
  ccenter("000"), ccenter("001"), ccenter("010"), ccenter("011"), 
  ccenter("100"), ccenter("101"), ccenter("110"), ccenter("111"),
  ccenter("000"), [], [], op("SSI"), [], op("FENCE"),  [], [], op("SYSCALL"),
  ccenter("001"), [], [], [],        [], op("CINVAL"), [], [], op("BREAKPT"),
  ccenter("010"), [], [], [],        [], op("CFETCH"), [], [], op("SPIN"),
  ccenter("011"), [], [], [],        [], [],           [], [], [],
  ccenter("100"), [], [], [],        [], [],           [], [], op("IRET"),
  ccenter("101"), [], [], [],        [], [],           [], [], op("LCTRL"),
  ccenter("110"), [], [], [],        [], [],           [], [], op("SCTRL"),
  ccenter("111"), [], [], [],        [], [],           [], [], op("WAIT"),
)


=== Format B

#table(
  columns: (auto, 1fr, 1.1fr, 1fr, 1fr, 1.1fr, 1fr, 1fr, 1.2fr),
  fill: (x, y) => if y == 0 or y == 1 or x == 0 {orbit_grey},
  table.cell("Minor", rowspan: 2, align: horizon + center), 
  table.cell("Major", colspan: 8, align: center),
  table.hline(stroke: 0pt),
  ccenter("000"), ccenter("001"), ccenter("010"), ccenter("011"),
  ccenter("100"), ccenter("101"), ccenter("110"), ccenter("111"),
  ccenter("000"), op("ADDI"),  op("ANDI"), op("SI"),  op("SULTI"), [],        [], [], [],
  ccenter("001"), op("SUBI"),  op("ORI"),  op("CB"),  op("SILTI"), [],        [], [], [],
  ccenter("010"), op("MULI"),  op("NORI"), op("REV"), op("SULEI"), [],        [], [], [],
  ccenter("011"), [],          op("XORI"), [],        op("SILEI"), [],        [], [], [],
  ccenter("100"), op("UDIVI"), op("CLZ"),  [],        op("SEQI"),  op("JLR"), [], [], [],
  ccenter("101"), op("IDIVI"), op("CTZ"),  [],        [],          op("JL"),  [], [], [],
  ccenter("110"), op("UREMI"), op("CSB"),  [],        [],          op("BZ"),  [], [], [],
  ccenter("111"), op("IREMI"), [],         [],        [],          op("BN"),  [], [], [],
)

=== Format C

#table(
  columns: (auto, 1fr, 1.1fr, 1fr, 1fr, 1.1fr, 1fr, 1fr, 1.2fr),
  fill: (x, y) => if y == 0 or y == 1 or x == 0 {orbit_grey},
  table.cell("Minor", rowspan: 2, align: horizon + center), 
  table.cell("Major", colspan: 8, align: center),
  table.hline(stroke: 0pt),
  ccenter("000"), ccenter("001"), ccenter("010"), ccenter("011"), 
  ccenter("100"), ccenter("101"), ccenter("110"), ccenter("111"),
  ccenter("000"), op("ADD"),  op("AND"),   op("USR"), op("SULT"), op("LW"),  op("SW"),  [], [],
  ccenter("001"), op("SUB"),  op("OR"),    op("ISR"), op("SILT"), op("LH"),  op("SH"),  [], [],
  ccenter("010"), op("MUL"),  op("NOR"),   op("ROR"), op("SULE"), op("LQ"),  op("SQ"),  [], [],
  ccenter("011"), [],         op("XOR"),   op("ROL"), op("SILE"), op("LB"),  op("SB"),  [], [],
  ccenter("100"), op("UDIV"), op("EXT"),   op("SL"),  op("SEQ"),  op("LLW"), op("SCW"), [], [],  
  ccenter("101"), op("IDIV"), op("DEP"),   [],        [],         op("LLH"), op("SCH"), [], [],  
  ccenter("110"), op("UREM"), op("UMULH"), [],        [],         op("LLQ"), op("SCQ"), [], [],     
  ccenter("111"), op("IREM"), op("IMULH"), [],        [],         op("LLB"), op("SCB"), [], [],   
)
