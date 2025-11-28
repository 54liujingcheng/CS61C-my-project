start:
addi x0, x0, 123
bne x0, x0, fail
bne x0, zero, fail
addi x1, x0, -1
addi x2, x0, 1
slt x3, x1, x2
beq x3, x0, fail
sltu x4, x1, x2
bne x4, x0, fail
lui x5, 0x12
srli x5, x5, 12
addi x6, x0, 18
bne x5, x6, fail
addi x31, x0, 1
jal x0, end
fail:
addi x31, x0, -1
end: