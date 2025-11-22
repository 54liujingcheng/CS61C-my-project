.globl write_matrix

.text
# ==============================================================================
# FUNCTION: Writes a matrix of integers into a binary file
# FILE FORMAT:
#   The first 8 bytes of the file will be two 4 byte ints representing the
#   numbers of rows and columns respectively. Every 4 bytes thereafter is an
#   element of the matrix in row-major order.
# Arguments:
#   a0 (char*) is the pointer to string representing the filename
#   a1 (int*)  is the pointer to the start of the matrix in memory
#   a2 (int)   is the number of rows in the matrix
#   a3 (int)   is the number of columns in the matrix
# Returns:
#   None
# Exceptions:
# - If you receive an fopen error or eof,
#   this function terminates the program with error code 93.
# - If you receive an fwrite error or eof,
#   this function terminates the program with error code 94.
# - If you receive an fclose error or eof,
#   this function terminates the program with error code 95.
# ==============================================================================
write_matrix:

    # Prologue
    addi sp, sp, -20
    sw s0, 0(sp)
    sw s1, 4(sp)
    sw s2, 8(sp)
    sw s3, 12(sp)
    sw ra, 16(sp) #rubbish
    mv s1, a1 #matrix
    mv s2, a2 #row
    mv s3, a3 #col
    mv a1, a0
    addi a2, x0, 1
    jal fopen
    bge a0, x0, FileOpen
    li a1 93
    lw s0, 0(sp)
    lw s1, 4(sp)
    lw s2, 8(sp)
    lw s3, 12(sp)
    lw ra, 16(sp) #rubbish
    addi sp, sp, 20
    j exit2
    FileOpen:
    mv s0, a0
    li a0, 8
    jal malloc
    sw s2, 0(a0)
    sw s3, 4(a0)
    mv a1, s0
    mv a2, a0
    addi sp, sp, -4
    sw a0, 0(sp)
    li a3 2
    li a4 4
    jal fwrite
    lw t0, 0(sp)
    addi sp, sp, 4
    li a3 2
    bne a0, a3, WriteFail
    mv a0, t0
    jal free
    mv a1, s0
    mv a2, s1
    mul a3, s2, s3
    li a4, 4
    jal fwrite
    mul a3, s2, s3
    bne a0, a3, WriteFail
    
    # Epilogue
    mv a0, s0
    jal fclose
    beq a0, x0, Fclose
    li a1, 95
    lw a0, 0(sp)
    lw ra, 12(sp)
    lw s0, 0(sp)
    lw s1, 4(sp)
    lw s2, 8(sp)
    lw s3, 12(sp)
    lw ra, 16(sp) #rubbish
    addi sp, sp, 20
    j exit2
    Fclose:
    lw a0, 0(sp)
    lw ra, 12(sp)
    lw s0, 0(sp)
    lw s1, 4(sp)
    lw s2, 8(sp)
    lw s3, 12(sp)
    lw ra, 16(sp) #rubbish
    addi sp, sp, 20

    ret
    
WriteFail:
li a1 94
lw s0, 0(sp)
lw s1, 4(sp)
lw s2, 8(sp)
lw s3, 12(sp)
lw ra, 16(sp) #rubbish
addi sp, sp, 20
j exit2