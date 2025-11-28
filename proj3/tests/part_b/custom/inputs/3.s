main:
addi x2, x0, 1024
addi x10, x0, 5
jal x1, sum
addi x5, x0, 15
bne x10, x5, fail
addi x31, x0, 1
jal x0, end
sum:
addi x2, x2, -8
sw x1, 4(x2)
sw x10, 0(x2)
addi x5, x0, 1
beq x10, x5, base_case
addi x10, x10, -1
jal x1, sum
lw x6, 0(x2)
add x10, x10, x6
end_sum:
lw x1, 4(x2)
addi x2, x2, 8
jalr x0, x1, 0
base_case:
addi x10, x0, 1
jal x0, end_sum
fail:
addi x31, x0, -1
end: