; R15 is return value register
; R0-2 are parameter registers (additional parameters
;   must be placed on the stack)
; 
; R0-6 are caller's responsibility to save
; R7-14  and FP are callee's responsability

PROMPT  .STR "Please enter the Fibonacci term you would like computed: "
TERM    .STR "Term "
RESULT  .STR " in the Fibonacci sequence is: "
NL      .STR "\n"

; Prompts user for input, and stores the resulting number in R14
ENTRY   LDA R3, PROMPT
        TRP #5
        TRP #2
        MOV R14, R3

        MOV R0, R3
        CALL fib

DONE    LDA R3, TERM 
        TRP #5 ; print "Term "
        MOV R3, R14
        TRP #1 ; print term number
        LDA R3, RESULT
        TRP #5 ; print " in the Fibonacci sequence is: "
        MOV R3, R15
        TRP #1 ; print the calcualted term
        LDa R3, NL
        TRP #5 ; print "\n"
        
        TRP #0
        

; fib(term) -> calcualted_term
fib     MOV R3, R0
        SUBI R3, R3, #1
        BGT R3, N_GT_1
        MOVI R15, #0 ; first term, so return 0
        RET

N_GT_1  SUBI R3, R3, #1
        BGT R3, N_GT_2
        MOVI R15, #1 ; second term, so return 1
        RET

N_GT_2  SUBI R0, R0, #1
        PSHR R0 ; save the parameter we care about
        CALL fib ; call fib(N - 1)
        POPR R0 ; restore the parameter we care about

        PSHR R15 ; store the result of the fib(N - 1) call on the stack

        SUBI R0, R0, #1
        CALL fib ; call fib(N - 2)

        POPR R3  ; retreive stored result of fib(N - 1)
        ADD R15, R15, R3 ; add result of the two fib() calls together
        RET ; return
