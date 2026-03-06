; comment for alignment
           jmp JMP_HERE 
SKIP_THIS  jmp END
JMP_HERE   mov r0, r1 ; comment
           movi r2, #4554
           lda r3, SKIP_THIS
           str r4, #4555
           ldr r5, #4556
           stb r6, #4557
           ldb r7, #4558
           add r8, r9, r10
           addi r11, r12, SKIP_THIS
           sub r13, r14, r15
           subi PC, SL, #255
           mul SB, SP, FP
           muli HP, r0, #254
           div r1, r2, r3
           sdiv r4, r5, r6
           divi r7, r8, #253
END        trp #0
