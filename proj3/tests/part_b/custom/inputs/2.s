start:
addi x1, x0, 10
addi x2, x1, 20
addi x3, x0, 30
bne x2, x3, fail
addi x4, x0, 0
beq x0, x0, jump_target
addi x4, x0, 100
jump_target:
addi x5, x0, 100
beq x4, x5, fail
la x1, return_addr
jalr x0, x1, 0
addi x31, x0, -1
jal x0, end
return_addr:
addi x30, x0, 1
jal x0, end
fail:
addi x30, x0, 0
end: