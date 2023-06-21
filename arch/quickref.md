# Aphelion ISA v0.2 Quick Reference

Registers - 64 bits wide

    rz      always reads zero, writes are ignored
    ra      general purpose
    rb      general purpose
    rc      general purpose
    rd      general purpose
    re      general purpose
    rf      general purpose
    rg      general purpose
    rh      general purpose
    ri      general purpose
    rj      general purpose
    rk      general purpose
    pc      program counter - points to current instruction in memory
    sp      stack pointer - stack grows down
    fp      frame pointer
    st      status register - contains bit flags and information about the processor state

Instructions - 32 bits wide

    name    parameters      description

    nop                     does nothing
    int     imm             trigger interrupt imm
    inv                     generate invalid opcode interrupt (equivalent to 'int 3')
    usr                     enter user mode - throws 'no permission' exception if used in user mode

    li      r1, imm         r1 = imm       * pseudo-instruction, expands to 'lli', 'lui', etc.
    lli     r1, imm         bits 0-15 of r1 = imm
    llis    r1, imm         r1 = i64(imm)
    lui     r1, imm         bits 16-31 of r1 = imm
    luis    r1, imm         r1 = i64(imm) << 16
    lti     r1, imm         bits 32-47 of r1 = imm
    ltis    r1, imm         r1 = i64(imm) << 32
    ltui    r1, imm         bits 48-63 of r1 = imm
    ltuis   r1, imm         r1 = i64(imm) << 48
    ld      r1, r2, imm     load 64-bit word at address (r2 + imm) to r1
    lbs     r1, r2, imm     load byte at address (r2 + imm) and sign-extend
    lb      r1, r2, imm     load byte at address (r2 + imm) to bits 0-7 of r1
    st      r1, r2, imm     store r1 as 64-bit word at address (r2 + imm)
    stb     r1, r2, imm     store bits 0-7 of r1 at address (r2 + imm)
    swp     r1, r2          swap values of r1 and r2
    mov     r1, r2          copy value of r1 to r2

    add     r1, r2, r3|imm  r1 = r2 + (r3|imm)          * pseudo-instruction, expands to 'addr' and 'addi'
    adc     r1, r2, r3|imm  r1 = r2 + (r3|imm) + carry  * pseudo-instruction, expands to 'adcr' and 'adci'
    sub     r1, r2, r3|imm  r1 = r2 - (r3|imm)          * pseudo-instruction, expands to 'subr' and 'subi'
    sbb     r1, r2, r3|imm  r1 = r2 - (r3|imm) - borrow * pseudo-instruction, expands to 'sbbr' and 'sbbi'
    mul     r1, r2, r3|imm  r1 = r2 * (r3|imm)          * pseudo-instruction, expands to 'mulr' and 'muli'
    div     r1, r2, r3|imm  r1 = r2 / (r3|imm)          * pseudo-instruction, expands to 'divr' and 'divi'

    addr    r1, r2, r3      r1 = r2 + r3
    addi    r1, r2, imm     r1 = r2 + imm
    adcr    r1, r2, r3      r1 = r2 + r3 + carry
    adci    r1, r2, imm     r1 = r2 + imm + carry
    subr    r1, r2, r3      r1 = r2 - r3
    subi    r1, r2, imm     r1 = r2 - imm
    sbbr    r1, r2, r3      r1 = r2 - r3 - borrow
    sbbi    r1, r2, imm     r1 = r2 - imm - borrow
    mulr    r1, r2, r3      r1 = r2 * r3
    muli    r1, r2, imm     r1 = r2 * imm
    divr    r1, r2, r3      r1 = r2 / r3
    divi    r1, r2, imm     r1 = r2 / imm

    and     r1, r2, r3|imm  r1 = r2 & (r3|imm)              * pseudo-instruction, expands to 'andr' and 'andi'
    or      r1, r2, r3|imm  r1 = r2 | (r3|imm)              * pseudo-instruction, expands to 'orr' and 'ori'
    nor     r1, r2, r3|imm  r1 = !(r2 | (r3|imm))           * pseudo-instruction, expands to 'norr' and 'nori'
    xor     r1, r2, r3|imm  r1 = r2 ^ (r3|imm)              * pseudo-instruction, expands to 'xorr' and 'xori'
    shl     r1, r2, r3|imm  r1 = r2 << (r3|imm)             * pseudo-instruction, expands to 'shlr' and 'shli'
    asr     r1, r2, r3|imm  r1 = r2 >> (r3|imm) (keep sign) * pseudo-instruction, expands to 'asrr' and 'asri'
    lsr     r1, r2, r3|imm  r1 = r2 >> (r3|imm)             * pseudo-instruction, expands to 'lsrr' and 'lsri'

    andr    r1, r2, r3      r1 = r2 & r3
    andi    r1, r2, imm     r1 = r2 & imm
    orr     r1, r2, r3      r1 = r2 | r3
    ori     r1, r2, imm     r1 = r2 | imm
    norr    r1, r2, r3      r1 = !(r2 | r3)
    nori    r1, r2, imm     r1 = !(r2 | imm)
    xorr    r1, r2, r3      r1 = r2 ^ r3
    xori    r1, r2, imm     r1 = r2 ^ imm
    shlr    r1, r2, r3      r1 = r2 << r3
    shli    r1, r2, imm     r1 = r2 << imm
    asrr    r1, r2, r3      r1 = r2 >> r3   (keep sign)
    asri    r1, r2, imm     r1 = r2 >> imm  (keep sign)
    lsrr    r1, r2, r3      r1 = r2 >> r3
    lsri    r1, r2, imm     r1 = r2 >> imm