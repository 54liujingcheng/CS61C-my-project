start:
addi x1, x0, 512
addi x2, x0, 123
sw x2, 0(x1)
lw x3, 0(x1)
bne x2, x3, fail
addi x4, x0, 456
sw x4, 4(x1)
lw x5, 4(x1)
bne x4, x5, fail
addi x6, x1, 4
addi x7, x0, 789
sw x7, -4(x6)
lw x8, 0(x1)
bne x7, x8, fail
addi x9, x0, 111
sw x9, 12(x1)
lw x10, 12(x1)
bne x9, x10, fail
addi x11, x1, 12
lw x12, 0(x11)
bne x9, x12, fail
addi x31, x0, 1
jal x0, end
fail:
addi x31, x0, -1
end: