.globl argmax

.text
# =================================================================
# FUNCTION: Given a int vector, return the index of the largest
#	element. If there are multiple, return the one
#	with the smallest index.
# Arguments:
# 	a0 (int*) is the pointer to the start of the vector
#	a1 (int)  is the # of elements in the vector
# Returns:
#	a0 (int)  is the first index of the largest element
# Exceptions:
# - If the length of the vector is less than 1,
#   this function terminates the program with error code 77.
# =================================================================
argmax:

    # Prologue
    blt x0, a1, loop_start
    li a1, 77
    j exit2
loop_start:
    addi t0, x0, 0 #index
    addi t1, a0, 0 #start of matrix
    addi a1, a1, -1
    addi t4, x0, 0 #ans
    lw a0, 0(t1) #max

loop_continue:
    blt a1, t0, loop_end
    slli t2, t0, 2
    add t2, t1, t2 #address of object
    lw t3, 0(t2)
    bge a0, t3, fin
    addi a0, t3, 0
    addi t4, t0, 0
    fin:
    addi t0, t0, 1
    j loop_continue

loop_end:
    addi a0, t4, 0

    # Epilogue


    ret
