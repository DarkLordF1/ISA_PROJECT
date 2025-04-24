# sort.asm — Bubble‐sort 16 words at 0x100–0x10F
# All branches in SIMP are: bxx rd, rs, rt, imm

    # load base address
    add   $gp,   $zero, $imm,   0x100    # R13 ← 0x100

    # i = 0; limit = 15
    add   $s0,   $zero, $imm,   0        # R10 ← 0
    add   $s1,   $zero, $imm,   15       # R11 ← 15

outer_loop:
    # if (i ≥ limit) goto outer_done
    add   $ra,   $zero, $imm,   outer_done
    bge   $ra,   $s0,   $s1,    0

    # inner_limit = limit − i
    sub   $s2,   $s1,   $s0,     0       # R12 ← R11 − R10

    # j = 0
    add   $t0,   $zero, $imm,    0       # R7 ← 0

    # uncond jump to inner_loop
    add   $ra,   $zero, $imm,   inner_loop
    beq   $ra,   $zero, $zero,  0

inner_loop:
    # if (j ≥ inner_limit) goto inner_done
    add   $ra,   $zero, $imm,   inner_done
    bge   $ra,   $t0,   $s2,    0

    # set shift‐amount = 2
    add   $imm,  $zero, $imm,   2       # R1 ← 2

    # load A[j] into $t1
    sll   $t1,   $t0,   $imm,    0       # t1 = j<<2
    add   $t1,   $gp,   $t1,     0       # t1 = base + offset
    lw    $t1,   $t1,   $zero,   0       # t1 = MEM[t1]

    # load A[j+1] into $t2
    add   $t2,   $t0,   $imm,    1       # t2 = j+1
    sll   $t2,   $t2,   $imm,    0       # t2 <<=2
    add   $t2,   $gp,   $t2,     0       # address
    lw    $t2,   $t2,   $zero,   0       # t2 = MEM[t2]

    # if (t1 ≤ t2) goto noswap
    add   $ra,   $zero, $imm,   noswap
    ble   $ra,   $t1,   $t2,    0

    # swap A[j]   ← old A[j+1]
    sw    $t2,   $gp,   $t1,    0

    # swap A[j+1] ← old A[j]
    sw    $t1,   $gp,   $t2,    0

noswap:
    # j++
    add   $t0,   $t0,   $imm,    1

    # back to inner_loop
    add   $ra,   $zero, $imm,   inner_loop
    beq   $ra,   $zero, $zero,  0

inner_done:
    # i++
    add   $s0,   $s0,   $imm,    1

    # back to outer_loop
    add   $ra,   $zero, $imm,   outer_loop
    beq   $ra,   $zero, $zero,  0

outer_done:
    halt  $zero, $zero, $zero,   0

# data: 16 words at 0x100–0x10F
.word 0x100  5
.word 0x101  3
.word 0x102  8
.word 0x103  1
.word 0x104  6
.word 0x105  7
.word 0x106  2
.word 0x107  9
.word 0x108  4
.word 0x109  0
.word 0x10A 15
.word 0x10B 11
.word 0x10C 14
.word 0x10D 10
.word 0x10E 12
.word 0x10F 13
