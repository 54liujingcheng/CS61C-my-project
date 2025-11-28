start:
auipc x1, 0
auipc x2, 0
addi x3, x0, 4
sub x4, x2, x1
bne x4, x3, fail
auipc x5, 0
jal x6, check_jal
addi x31, x0, -1
jal x0, end
check_jal:
sub x7, x6, x5
addi x8, x0, 8
bne x7, x8, fail
addi x31, x0, 1
jal x0, end
fail:
addi x31, x0, -1
end: