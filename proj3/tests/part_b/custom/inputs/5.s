start:
addi x1, x0, -2
srai x2, x1, 1
addi x3, x0, -1
bne x2, x3, fail
srli x4, x1, 1
addi x5, x0, 1
slli x5, x5, 31
addi x6, x0, -1
xor x5, x5, x6
bne x4, x5, fail
addi x7, x0, 240
addi x8, x0, 15
and x9, x7, x8
bne x9, x0, fail
or x9, x7, x8
addi x10, x0, 255
bne x9, x10, fail
xor x9, x7, x10
bne x9, x8, fail
addi x11, x0, 10
addi x12, x0, 5
bge x12, x11, fail
bge x11, x12, pass
fail:
addi x31, x0, -1
jal x0, end
pass:
addi x31, x0, 1
end: