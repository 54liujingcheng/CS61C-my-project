start:
addi x10, x0, 512
sw x0, 0(x10)
addi x1, x0, -1
sb x1, 0(x10)
lb x2, 0(x10)
bne x1, x2, fail
sb x1, 1(x10)
lb x2, 1(x10)
bne x1, x2, fail
sb x1, 2(x10)
lb x2, 2(x10)
bne x1, x2, fail
sb x1, 3(x10)
lb x2, 3(x10)
bne x1, x2, fail
lw x4, 0(x10)
bne x4, x1, fail
sw x0, 4(x10)
sh x1, 4(x10)
lh x2, 4(x10)
bne x1, x2, fail
lb x3, 4(x10)
bne x1, x3, fail
lb x3, 5(x10)
bne x1, x3, fail
lb x3, 6(x10)
bne x3, x0, fail
sh x1, 6(x10)
lh x2, 6(x10)
bne x1, x2, fail
lw x4, 4(x10)
bne x4, x1, fail
sw x0, 8(x10)
addi x1, x0, 17
sb x1, 8(x10)
addi x1, x0, 34
sb x1, 9(x10)
addi x1, x0, 51
sb x1, 10(x10)
addi x1, x0, 68
sb x1, 11(x10)
lw x2, 8(x10)
addi x3, x0, 68
slli x3, x3, 8
addi x3, x3, 51
slli x3, x3, 8
addi x3, x3, 34
slli x3, x3, 8
addi x3, x3, 17
bne x2, x3, fail
addi x4, x0, -1
sh x4, 10(x10)
lw x2, 8(x10)
addi x3, x0, -1
slli x3, x3, 16
addi x5, x0, 34
slli x5, x5, 8
add x3, x3, x5
addi x5, x0, 17
add x3, x3, x5
bne x2, x3, fail
jal x0, pass
fail:
addi x31, x0, -1
jal x0, end
pass:
addi x31, x0, 1
end: