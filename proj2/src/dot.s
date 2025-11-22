.globl dot

.text
# =======================================================
# FUNCTION: Dot product of 2 int vectors
# Arguments:
#   a0 (int*) is the pointer to the start of v0
#   a1 (int*) is the pointer to the start of v1
#   a2 (int)  is the length of the vectors
#   a3 (int)  is the stride of v0
#   a4 (int)  is the stride of v1
# Returns:
#   a0 (int)  is the dot product of v0 and v1
# Exceptions:
# - If the length of the vector is less than 1,
#   this function terminates the program with error code 75.
# - If the stride of either vector is less than 1,
#   this function terminates the program with error code 76.
# =======================================================
dot:

    # Prologue
    blt x0, a2, check1
    li a1, 75
    j exit2
    check1:
    blt x0, a3, check2
    li a1, 76
    j exit2
    check2:
    blt x0, a4, loop_start
    li a1, 76
    j exit2
loop_start:
    addi t0, x0, 0 #index
    addi t1, x0, 0 #ans
    addi a2, a2, -1

loop_continue:
    blt a2, t0, loop_end
    mul t2, t0, a3
    slli t2, t2, 2 
    add t2, a0, t2 #address x0
    mul t3, t0, a4
    slli t3, t3, 2
    add t3, a1, t3
    lw t4, 0(t2)
    lw t5, 0(t3)
    mul t4, t4, t5
    add t1, t1, t4
    addi t0, t0, 1
    j loop_continue

loop_end:
    addi a0, t1, 0

    # Epilogue

    
    ret
