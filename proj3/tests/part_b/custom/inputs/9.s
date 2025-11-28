start:
addi x1, x0, 100
addi x2, x0, 256
sw x1, 0(x2)
lw x3, 0(x2)
add x4, x3, x1
addi x5, x0, 200
bne x4, x5, fail
addi x6, x0, 10
sw x6, 4(x2)
lw x7, 4(x2)
addi x7, x7, 1
addi x8, x0, 11
bne x7, x8, fail
jal x0, pass
fail:
addi x31, x0, -1
jal x0, end
pass:
addi x31, x0, 1
end: