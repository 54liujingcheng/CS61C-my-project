.globl classify

.text
classify:
    # =====================================
    # COMMAND LINE ARGUMENTS
    # =====================================
    # Args:
    #   a0 (int)    argc
    #   a1 (char**) argv
    #   a2 (int)    print_classification, if this is zero, 
    #               you should print the classification. Otherwise,
    #               this function should not print ANYTHING.
    # Returns:
    #   a0 (int)    Classification
    # Exceptions:
    # - If there are an incorrect number of command line args,
    #   this function terminates the program with exit code 89.
    # - If malloc fails, this function terminats the program with exit code 88.
    #
    # Usage:
    #   main.s <M0_PATH> <M1_PATH> <INPUT_PATH> <OUTPUT_PATH>






	# =====================================
    # LOAD MATRICES
    # =====================================
    # Prologue
    addi sp, sp, -48
    sw s0, 0(sp)
    sw s1, 4(sp)
    sw s2, 8(sp)
    sw s3, 12(sp)
    sw s4, 16(sp)
    sw s5, 20(sp)
    sw s6, 24(sp)
    sw s7, 28(sp)
    sw s8, 32(sp)
    sw s9, 36(sp)
    sw ra, 40(sp) #rubbish
    sw s10, 44(sp)
    mv s1, a1 #char** of addr
    mv s2, a2 #print_flag
    addi t0, x0, 5
    beq a0, t0, Argc
    li a1 89
    lw s0, 0(sp) 
    lw s1, 4(sp) 
    lw s2, 8(sp) 
    lw s3, 12(sp) #free
    lw s4, 16(sp) #free
    lw s5, 20(sp) #free
    lw s6, 24(sp) #free
    lw s7, 28(sp) #free
    lw s8, 32(sp) #free
    lw s9, 36(sp) #free
    lw ra, 40(sp) #rubbish
    lw s10, 44(sp) 
    addi sp, sp, 48
    j exit2
    Argc:
    

    # Load pretrained m0
    li a0 8
    jal malloc
    beq a0, x0, MallocFail
    mv s3, a0 #row addr and col addr of m0
    addi a1, s3, 0
    addi a2, s3, 4
    lw a0, 4(s1) #argv[1]
    jal read_matrix
    mv s4, a0 #m0 addr

    # Load pretrained m1
    li a0 8
    jal malloc
    beq a0, x0, MallocFail
    mv s5, a0 #row addr and col addr of m1
    addi a1, s5, 0
    addi a2, s5, 4
    lw a0, 8(s1) #argv[2]
    jal read_matrix
    mv s6, a0 #m1 addr

    # Load input matrix
    li a0 8
    jal malloc
    beq a0, x0, MallocFail
    mv s7, a0 #row addr and col addr of input
    addi a1, s7, 0
    addi a2, s7, 4
    lw a0, 12(s1) #argv[3]
    jal read_matrix
    mv s8, a0 #input_matrix addr

    # =====================================
    # RUN LAYERS
    # =====================================
    # 1. LINEAR LAYER:    m0 * input
    # 2. NONLINEAR LAYER: ReLU(m0 * input)
    # 3. LINEAR LAYER:    m1 * ReLU(m0 * input)
    lw a1, 0(s3)
    lw a5, 4(s7)
    mul a0, a1, a5
    slli a0, a0, 2
    jal malloc
    beq a0, x0, MallocFail
    mv a6, a0
    mv s9, a0 #linear layer 1
    mv a0, s4
    mv a3, s8
    lw a1, 0(s3)
    lw a2, 4(s3)
    lw a4, 0(s7)
    lw a5, 4(s7)
    jal matmul

    lw t2, 0(s3)
    lw t4, 4(s7)
    mul a1, t2, t4
    mv a0, s9
    jal relu #non linear layer

    
    lw a5, 4(s7) #col
    
    lw a1, 0(s5) #row
    
    mul a0, a1, a5
    slli a0, a0, 2
    jal malloc
    beq a0, x0, MallocFail
    lw a1, 0(s5) #row
    lw a5, 4(s7) #col
    lw a4, 0(s3)
    lw a2, 4(s5)
    mv a3, s9
    mv a6, a0
    mv s10, a0
    mv a0, s6
    jal matmul
    mv a0, s9
    jal free
    mv s9, s10 #linear layer 2

    # =====================================
    # WRITE OUTPUT
    # =====================================
    # Write output matrix
    lw a0, 16(s1) #argv[4]
    lw a2, 0(s5) #row
    lw a3, 4(s7) #col
    mv a1, s9
    jal write_matrix
    # =====================================
    # CALCULATE CLASSIFICATION/LABEL
    # =====================================
    # Call argmax
    lw a2, 0(s5) #row
    lw a3, 4(s7) #col
    mul a1, a2, a3
    mv a0, s9
    jal argmax

    # Print classification
    mv s10, a0
    bne s2, x0, NoPrint
    mv a1, s10
    jal print_int
    # Print newline afterwards for clarity
    li a1 '\n'
    jal print_char

    NoPrint:
    mv a0, s3
    jal free
    mv a0, s4
    jal free
    mv a0, s5
    jal free
    mv a0, s6
    jal free
    mv a0, s7
    jal free
    mv a0, s8
    jal free
    mv a0, s9
    jal free
    mv a0, s10
    lw s0, 0(sp)
    lw s1, 4(sp)
    lw s2, 8(sp)
    lw s3, 12(sp)
    lw s4, 16(sp)
    lw s5, 20(sp)
    lw s6, 24(sp)
    lw s7, 28(sp)
    lw s8, 32(sp)
    lw s9, 36(sp)
    lw ra, 40(sp) #rubbish
    lw s10, 44(sp)
    addi sp, sp, 48
    ret
    
MallocFail:
li a1 88
lw s0, 0(sp)
lw s1, 4(sp)
lw s2, 8(sp)
lw s3, 12(sp)
lw s4, 16(sp)
lw s5, 20(sp)
lw s6, 24(sp)
lw s7, 28(sp)
lw s8, 32(sp)
lw s9, 36(sp)
lw ra, 40(sp) #rubbish
lw s10, 44(sp)
addi sp, sp, 48
j exit2