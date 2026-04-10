; comment for alignment
           jmp JMP_HERE 
SKIP_THIS  jmp END
JMP_HERE   mov r0, r1 ; comment
           movi r2, #4554
           lda r3, SKIP_THIS
           str r4, SKIP_THIS
           ldr r5, JMP_HERE
           stb r6, SKIP_THIS
           ldb r7, JMP_HERE
           add r8, r9, r10
           addi r11, r12, #12
           sub r13, r14, r15
           subi PC, SL, #255
           mul SB, SP, FP
           muli HP, r0, #254
           div r1, r2, r3
           sdiv r4, r5, r6
           divi r7, r8, #253
END        trp #0
