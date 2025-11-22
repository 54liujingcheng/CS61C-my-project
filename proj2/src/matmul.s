.globl matmul

.text
# =======================================================
# FUNCTION: Matrix Multiplication of 2 integer matrices
# 	d = matmul(m0, m1)
# Arguments:
# 	a0 (int*)  is the pointer to the start of m0 
#	a1 (int)   is the # of rows (height) of m0
#	a2 (int)   is the # of columns (width) of m0
#	a3 (int*)  is the pointer to the start of m1
# 	a4 (int)   is the # of rows (height) of m1
#	a5 (int)   is the # of columns (width) of m1
#	a6 (int*)  is the pointer to the the start of d
# Returns:
#	None (void), sets d = matmul(m0, m1)
# Exceptions:
#   Make sure to check in top to bottom order!
#   - If the dimensions of m0 do not make sense,
#     this function terminates the program with exit code 72.
#   - If the dimensions of m1 do not make sense,
#     this function terminates the program with exit code 73.
#   - If the dimensions of m0 and m1 don't match,
#     this function terminates the program with exit code 74.
# =======================================================
matmul:

    # Error checks
    blt x0, a1, check1
    li a1, 72
    j exit2
    check1:
    blt x0, a2, check2
    li a1, 72
    j exit2
    check2:
    blt x0, a5, check3
    li a1, 73
    j exit2
    check3:
    blt x0, a4, check4
    li a1, 73
    j exit2
    check4:
    beq a2, a4, check5
    li a1, 74
    j exit2
    check5:
    # Prologue
    addi a1, a1, -1
    addi a5, a5, -1
    addi sp, sp, -32
    sw ra, 0(sp)
    sw a0, 4(sp)
    sw a1, 8(sp)
    sw a2, 12(sp)
    sw a3, 16(sp)
    sw a4, 20(sp)
    sw a5, 24(sp)
    sw a6, 28(sp)
    addi t0, x0, 0 #index of ml
    addi t1, x0, 0 #index of mr
    addi t2, x0, 0 #index of d
outer_loop_start:
    addi t5, a1, 1
    bge t0, t5, outer_loop_end

inner_loop_start:
    addi t5, a5, 1
    bge t1, t5, inner_loop_end
    mul t3, t0, a2
    slli t3, t3, 2
    add t3, a0, t3 #address of ml
    addi t4, t1, 0
    slli t4, t4, 2
    add t4, a3, t4 #address of mr
    addi a0, t3, 0
    addi a1, t4, 0
    li a3, 1
    addi a4, a5, 1
    addi sp, sp, -12
    sw t0, 0(sp)
    sw t1, 4(sp)
    sw t2, 8(sp)
    jal dot
    lw t2, 8(sp)
    lw t1, 4(sp)
    lw t0, 0(sp)
    addi sp, sp, 12
    lw ra, 0(sp)
    lw a1, 8(sp)
    lw a2, 12(sp)
    lw a3, 16(sp)
    lw a4, 20(sp)
    lw a5, 24(sp)
    lw a6, 28(sp)
    add t3, t2, a6 #address of d
    addi t2, t2, 4
    sw a0, 0(t3)
    lw a0, 4(sp)
    addi t1, t1, 1
    j inner_loop_start
inner_loop_end:
    addi t1, x0, 0


    addi t0, t0, 1
    j outer_loop_start
outer_loop_end:


    # Epilogue
    addi sp, sp, 32
    
    ret
