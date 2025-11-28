start:
addi x1, x0, 1
addi x2, x0, -31
sll x3, x1, x2
addi x4, x0, 2
bne x3, x4, fail
addi x5, x0, -4
sra x6, x5, x2
addi x7, x0, -2
bne x6, x7, fail
addi x8, x0, 33
addi x9, x0, -1
srl x10, x9, x8
lui x11, 524288
addi x11, x11, -1
bne x10, x11, fail
jal x0, pass
fail:
addi x31, x0, -1
jal x0, end
pass:
addi x31, x0, 1
end: