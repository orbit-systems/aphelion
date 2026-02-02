executable section "text"
    lw l0, [l1 + l2 + 511]
    lw l0, [l1 + 511]
    lw l0, [l1 + l2]
    lw l0, [l1]
    lh l0, [l1 + l2 + 511]
    lh l0, [l1 + 511]
    lh l0, [l1 + l2]
    lh l0, [l1]
    lq l0, [l1 + l2 + 511]
    lq l0, [l1 + 511]
    lq l0, [l1 + l2]
    lq l0, [l1]
    lb l0, [l1 + l2 + 511]
    lb l0, [l1 + 511]
    lb l0, [l1 + l2]
    lb l0, [l1]

    llw l0, [l1 + l2 + 511]
    llw l0, [l1 + 511]
    llw l0, [l1 + l2]
    llw l0, [l1]
    llh l0, [l1 + l2 + 511]
    llh l0, [l1 + 511]
    llh l0, [l1 + l2]
    llh l0, [l1]
    llq l0, [l1 + l2 + 511]
    llq l0, [l1 + 511]
    llq l0, [l1 + l2]
    llq l0, [l1]
    llb l0, [l1 + l2 + 511]
    llb l0, [l1 + 511]
    llb l0, [l1 + l2]
    llb l0, [l1]

    sw l0, [l1 + l2 + 511]
    sw l0, [l1 + 511]
    sw l0, [l1 + l2]
    sw l0, [l1]
    sh l0, [l1 + l2 + 511]
    sh l0, [l1 + 511]
    sh l0, [l1 + l2]
    sh l0, [l1]
    sq l0, [l1 + l2 + 511]
    sq l0, [l1 + 511]
    sq l0, [l1 + l2]
    sq l0, [l1]
    sb l0, [l1 + l2 + 511]
    sb l0, [l1 + 511]
    sb l0, [l1 + l2]
    sb l0, [l1]

    scw l2, l0, [l1 + 511]
    scw l2, l0, [l1]
    sch l2, l0, [l1 + 511]
    sch l2, l0, [l1]
    scq l2, l0, [l1 + 511]
    scq l2, l0, [l1]
    scb l2, l0, [l1 + 511]
    scb l2, l0, [l1]

    fence.s
    fence.l
    fence.sl

    cinval.block   l0
    cinval.page    l0
    cinval.all     l0
    cinval.i.block l0
    cinval.i.page  l0
    cinval.i.all   l0
    cinval.d.block l0
    cinval.d.page  l0
    cinval.d.all   l0

    cfetch.s   l0
    cfetch.l   l0
    cfetch.i   l0
    cfetch.sl  l0
    cfetch.li  l0
    cfetch.si  l0
    cfetch.sli l0

    ssi   l0, 0xffff, 0
    ssi   l0, 0xffff, 16
    ssi   l0, 0xffff, 32
    ssi   l0, 0xffff, 48
    ssi.c l0, 0xffff, 0
    ssi.c l0, 0xffff, 16
    ssi.c l0, 0xffff, 32
    ssi.c l0, 0xffff, 48

    add  l0, l1, l2
    add  l0, l1, l2, 511
    sub  l0, l1, l2
    sub  l0, l1, l2, 511
    mul  l0, l1, l2
    mul  l0, l1, l2, 511
    umulh l0, l1, l2
    umulh l0, l1, l2, 511
    imulh l0, l1, l2
    imulh l0, l1, l2, 511
    udiv  l0, l1, l2
    udiv  l0, l1, l2, 511
    idiv  l0, l1, l2
    idiv  l0, l1, l2, 511
    urem  l0, l1, l2
    urem  l0, l1, l2, 511
    irem  l0, l1, l2
    irem  l0, l1, l2, 511

    addi  l0, l1, 16383
    subi  l0, l1, 16383
    muli  l0, l1, 16383
    udivi l0, l1, 16383
    idivi l0, l1, 16383
    uremi l0, l1, 16383
    iremi l0, l1, 16383

    and l0, l1, l2
    and l0, l1, l2, 511
    or  l0, l1, l2
    or  l0, l1, l2, 511
    nor l0, l1, l2
    nor l0, l1, l2, 511
    xor l0, l1, l2
    xor l0, l1, l2, 511

    andi l0, l1, 16383
    ori  l0, l1, 16383
    nori l0, l1, 16383
    xori l0, l1, 16383

    sl   l0, l1, l2, 63
    sl   l0, l1, 63
    sl   l0, l1, l2
    usr  l0, l1, l2, 63
    usr  l0, l1, 63
    usr  l0, l1, l2
    isr  l0, l1, l2, 63
    isr  l0, l1, 63
    isr  l0, l1, l2
    si.u l0, l1, 63, 63
    si.i l0, l1, 63, 63
    cb   l0, l1, 63, 63
    ror  l0, l1, l2, 63
    ror  l0, l1, 63
    ror  l0, l1, l2
    rol  l0, l1, l2, 63
    rol  l0, l1, 63
    rol  l0, l1, l2

    rev     l0, l1, 0b111111
    rev.h   l0, l1 // shorthand
    rev.q   l0, l1 // shorthand
    rev.b   l0, l1 // shorthand
    rev.bit l0, l1 // shorthand

    csb l0, l1
    clz l0, l1
    ctz l0, l1

    ext l0, l1, l2
    dep l0, l1, l2

    seq  l0, l1, l2
    sult l0, l1, l2
    silt l0, l1, l2
    sule l0, l1, l2
    sile l0, l1, l2

    seqi  l0, l1, 16383
    sulti l0, l1, 16383
    silti l0, l1, 8191
    silti l0, l1, -8192
    sulei l0, l1, 16383
    silei l0, l1, 8191
    silei l0, l1, -8192

label:
    bz l0, label
    bn l0, label

    jl l1, l0
    jl l1, l0, 8191
    jl l1, l0, -8192
    jlr l1, l0
    jlr l1, l0, 8191
    jlr l1, l0, -8192

    syscall
    breakpt
    wait
    spin
    iret
    lctrl l0, intstat
    sctrl intstat, l0

    call l1, l2, label
    call l1, label
    call label

    fcall l1, l2, label
    fcall l1, label
    fcall label

    ret
    nop
    mov l1, l2
    li l1, 0xBAAD_F00D_DEAD_BEEF
