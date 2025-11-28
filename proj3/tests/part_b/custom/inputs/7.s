start:
addi x1, x0, -1
addi x2, x0, 1
blt x2, x1, fail
blt x1, x2, test_bgeu
jal x0, fail
test_bgeu:
bgeu x1, x2, test_bltu
jal x0, fail
test_bltu:
bltu x2, x1, pass
jal x0, fail
fail:
addi x31, x0, -1
jal x0, end
pass:
addi x31, x0, 1
end: