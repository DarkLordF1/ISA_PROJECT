# disktest.asm — sum sectors 0–3 into sector 4

    # constants
    add   $imm,   $zero, $imm, 128
    add   $one,   $zero, $imm, 1

    # buffers (word addresses)
    add   $s0,    $zero, $imm, 0x200  # buf0
    add   $s1,    $zero, $imm, 0x280  # buf1
    add   $s2,    $zero, $imm, 0x300  # buf2
    add   $s3,    $zero, $imm, 0x380  # buf3
    add   $s4,    $zero, $imm, 0x400  # buf4

    # I/O register indices
    add   $rStat, $zero, $imm, 17     # diskstatus
    add   $rSec,  $zero, $imm, 15     # disksector
    add   $rBuf,  $zero, $imm, 16     # diskbuffer
    add   $rCmd,  $zero, $imm, 14     # diskcmd
    add   $rIrq1, $zero, $imm, 5      # irq1status

    # — Read sectors 0–3 into buf0–3 —
    add   $t0,    $zero, $imm,   0
read_loop:
    bge   $t0,    $imm,   read_done

wait_free:
    in    $t1,    $rStat, $zero,  0
    bne   $t1,    $zero,  wait_free

    out   $t0,    $rSec,  $zero,  0
    sll   $t2,    $t0,    $imm,   7   # t2 = t0 * 128
    add   $t2,    $s0,    $t2,    0
    out   $t2,    $rBuf,  $zero,  0

    add   $t3,    $zero, $imm,     1
    out   $t3,    $rCmd,  $zero,  0

wait_irq:
    in    $t1,    $rIrq1, $zero,  0
    beq   $t1,    $zero,  wait_irq
    out   $zero,  $rIrq1, $zero,  0

    add   $t0,    $t0,    $one,    0
    beq   $zero,  $zero,  read_loop
read_done:

    # — Sum into buf4 —
    add   $t0,    $zero,  $zero,  0
sum_loop:
    bge   $t0,    $imm,    sum_done
    lw    $t1,    $s0,     $t0,    0
    lw    $t2,    $s1,     $t0,    0
    lw    $t3,    $s2,     $t0,    0
    lw    $t4,    $s3,     $t0,    0
    add   $t5,    $t1,     $t2,    0
    add   $t5,    $t5,     $t3,    0
    add   $t5,    $t5,     $t4,    0
    sw    $t5,    $s4,     $t0,    0
    add   $t0,    $t0,     $one,   0
    beq   $zero,  $zero,   sum_loop
sum_done:

    # — Write buf4 back to sector 4 —
wait_free2:
    in    $t1,    $rStat,  $zero,  0
    bne   $t1,    $zero,   wait_free2

    add   $t0,    $zero,   $imm,   4
    out   $t0,    $rSec,   $zero,  0
    out   $s4,    $rBuf,   $zero,  0

    add   $t0,    $zero,   $imm,   2
    out   $t0,    $rCmd,   $zero,  0

wait_irq2:
    in    $t1,    $rIrq1,  $zero,  0
    beq   $t1,    $zero,   wait_irq2
    out   $zero,  $rIrq1,  $zero,  0

    halt  $zero,  $zero,   $zero,  0
