# factorial.asm — compute n! recursively, store result at MEM[0x100]

    # load argument n
    add   $a0,   $zero, $imm, 5

    # load address of fact() into $t0, then call
    add   $t0,   $zero, $imm, fact
    jal   $ra,   $t0,   $zero, 0

    # on return, $v0 = n!
    add   $gp,   $zero, $imm, 0x100
    sw    $v0,   $gp,   $zero,  0

    halt  $zero, $zero, $zero,  0

fact:
    # if n == 0 return 1
    beq   $a0,   $zero, base_case

    # save n in $t1
    add   $t1,   $a0,   $zero,  0

    # a0 = n-1
    sub   $a0,   $a0,   $zero,  1

    # recursive call: fact(n-1)
    add   $t0,   $zero, $imm, fact
    jal   $ra,   $t0,   $zero,  0

    # multiply returned value by saved n
    mul   $v0,   $t1,   $v0,    0

    # return via R[$ra]
    beq   $ra,   $zero, $zero,  0

base_case:
    add   $v0,   $zero, $imm,   1
    beq   $ra,   $zero, $zero,  0
