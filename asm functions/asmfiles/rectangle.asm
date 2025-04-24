# rectangle.asm — fill a white rectangle using four corner indices

    # read A–D from MEM[0x100–0x103]
    add   $imm,   $zero, $imm, 0x100
    lw    $s0,    $zero, $imm, 0    # A
    add   $imm,   $zero, $imm, 0x101
    lw    $s1,    $zero, $imm, 0    # B (unused here)
    add   $imm,   $zero, $imm, 0x102
    lw    $s2,    $zero, $imm, 0    # C
    add   $imm,   $zero, $imm, 0x103
    lw    $s3,    $zero, $imm, 0    # D (unused here)

    # split A → (r0,c0), C → (r1,c1)
    add   $imm,   $zero, $imm,   8
    srl   $r0,    $s0,    $imm,   0
    sll   $t0,    $r0,    $imm,   0
    sub   $c0,    $s0,    $t0,    0

    srl   $r1,    $s2,    $imm,   0
    sll   $t1,    $r1,    $imm,   0
    sub   $c1,    $s2,    $t1,    0

    # prepare I/O regs & constants
    add   $rAddr, $zero, $imm, 20
    add   $rData, $zero, $imm, 21
    add   $rCmd,  $zero, $imm, 22
    add   $white, $zero, $imm, 255
    add   $one,   $zero, $imm, 1

    # outer loop y=r0..r1
    add   $y,     $zero, $r0,   0
outer_y:
    bgt   $y,     $r1,    done

    # inner loop x=c0..c1
    add   $x,     $zero, $c0,   0
inner_x:
    bgt   $x,     $c1,    next_row

    # idx = (y<<8) + x
    sll   $t2,    $y,    $imm,   0
    add   $pix,   $t2,   $x,     0

    out   $pix,   $rAddr, $zero, 0
    out   $white, $rData, $zero, 0
    out   $one,   $rCmd,  $zero, 0

    add   $x,     $x,     $one,   0
    beq   $zero, $zero,  inner_x
next_row:
    add   $y,     $y,     $one,   0
    beq   $zero, $zero,  outer_y
done:
    halt  $zero, $zero, $zero, 0

# corner data (replace with your actual indices)
.word 0x100  0x0100
.word 0x101  0x0F00
.word 0x102  0x0F0F
.word 0x103  0x000F
