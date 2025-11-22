.globl relu

.text
# ==============================================================================
# FUNCTION: Performs an inplace element-wise ReLU on an array of ints
# Arguments:
# 	a0 (int*) is the pointer to the array
#	a1 (int)  is the # of elements in the array
# Returns:
#	None
# Exceptions:
# - If the length of the vector is less than 1,
#   this function terminates the program with error code 78.
# ==============================================================================
relu:
    # Prologue
    blt x0, a1, loop_start
    li a1 78
    j exit2
loop_start:
    addi t0, x0, 0 #index
    addi t1, a0, 0 #start of matrix
    addi a1, a1, -1
    
    
    
    
    
loop_continue:
    blt a1, t0, loop_end
    slli t2, t0, 2
    add t2, t1, t2 #address of object
    lw a0, 0(t2)
    bge a0, x0, fin
    addi a0, x0, 0
    fin:
    sw a0, 0(t2)
    addi t0, t0, 1
    j loop_continue
loop_end:


    # Epilogue

    
	ret
