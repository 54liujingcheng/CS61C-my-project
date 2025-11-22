.globl read_matrix

.text
# ==============================================================================
# FUNCTION: Allocates memory and reads in a binary file as a matrix of integers
#
# FILE FORMAT:
#   The first 8 bytes are two 4 byte ints representing the # of rows and columns
#   in the matrix. Every 4 bytes afterwards is an element of the matrix in
#   row-major order.
# Arguments:
#   a0 (char*) is the pointer to string representing the filename
#   a1 (int*)  is a pointer to an integer, we will set it to the number of rows
#   a2 (int*)  is a pointer to an integer, we will set it to the number of columns
# Returns:
#   a0 (int*)  is the pointer to the matrix in memory
# Exceptions:
# - If malloc returns an error,
#   this function terminates the program with error code 88.
# - If you receive an fopen error or eof, 
#   this function terminates the program with error code 90.
# - If you receive an fread error or eof,
#   this function terminates the program with error code 91.
# - If you receive an fclose error or eof,
#   this function terminates the program with error code 92.
# ==============================================================================
read_matrix:

    # Prologue
	addi sp, sp, -24
    sw s0, 0(sp)
    sw s1, 4(sp)
    sw s2, 8(sp)
    sw ra, 12(sp)
    sw s3, 16(sp)
    sw s4, 20(sp)
    mv s0, a0
    mv s1, a1
    mv s2, a2
    mv a1, s0
    addi a2, x0, 0
    jal fopen
    mv s0, a0
    bge a0, x0, FileOpen
    li a1 90
    lw s0, 0(sp)
    lw s1, 4(sp)
    lw s2, 8(sp)
    lw ra, 12(sp)
    lw s3, 16(sp)
    lw s4, 20(sp)
    addi sp, sp, 24
    j exit2
    FileOpen:
    mv a1, s0
    mv a2, s1 #row
    li a3 4
    jal fread
    li a3 4
    bne a0, a3, ReadFail
    lw s3, 0(s1) #load row
    mv a1, s0
    mv a2, s2
    li a3 4
    jal fread
    li a3 4
    bne a0, a3, ReadFail
    lw s4, 0(s2) #load col
    mul a0, s3, s4
    slli a0, a0, 2
    jal malloc #a0 -> matrix
    bne a0, x0, Malloc
    li a1 88
    lw s0, 0(sp)
    lw s1, 4(sp)
    lw s2, 8(sp)
    lw ra, 12(sp)
    lw s3, 16(sp)
    lw s4, 20(sp)
    addi sp, sp, 24
    j exit2
    Malloc:
    mv a2, a0
    mv a1, s0
    mul a3, s3, s4
    slli a3, a3, 2
    mv s2, a0
    jal fread
    mul a3, s3, s4
    slli a3, a3, 2
    bne a0, a3, ReadFail
    # Epilogue
    mv a0, s0
    jal fclose
    beq a0, x0, Fclose
    li a1, 92
    lw s0, 0(sp)
    lw s1, 4(sp)
    lw s2, 8(sp)
    lw ra, 12(sp)
    lw s3, 16(sp)
    lw s4, 20(sp)
    addi sp, sp, 24
    j exit2
    Fclose:
    mv a0, s2
    lw s0, 0(sp)
    lw s1, 4(sp)
    lw s2, 8(sp)
    lw ra, 12(sp)
    lw s3, 16(sp)
    lw s4, 20(sp)
    addi sp, sp, 24
    ret

ReadFail:
li a1 91
lw s0, 0(sp)
lw s1, 4(sp)
lw s2, 8(sp)
lw ra, 12(sp)
lw s3, 16(sp)
lw s4, 20(sp)
addi sp, sp, 24
j exit2