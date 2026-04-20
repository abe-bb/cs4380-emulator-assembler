; R15 is return value register
; R0-2 are parameter registers (additional parameters
;   must be placed on the stack)
; 
; R0-6 are caller's responsibility to save
; R7-14  and FP are callee's responsability


PROMPT1 .STR "Please enter an integer dividend: "
PROMPT2 .STR "Please enter an integer divisor: "
DIV     .STR " divided by "
RESULTS .STR " results in a remainder of: "
NL      .STR "\n"

; Get user input
ENTRY   LDA R3, PROMPT1
        TRP #5 
        TRP #2
        MOV R0, R3 ; dividend parameter

        LDA R3, PROMPT2
        TRP #5
        TRP #2
        MOV R1, R3 ; divisor parameter

; Call mod() function
        MOV R7, R0
        MOV R8, R1
        CALL mod

; Output results
        MOV R3, R7
        TRP #1 ; print dividend
        LDA R3, DIV
        TRP #5 ; print DIV
        MOV R3, R8
        TRP #1 ; print divisor
        LDA R3, RESULTS
        TRP #5 ; print RESULTS
        MOV R3, R15
        TRP #1 ; print modulus
        LDA R3, NL
        TRP #5 ; print \n

EXIT    TRP #0
        
        


; mod(dividend, divisor) -> modulus
mod     SDIV R3, R0, R1 ; quotient
        MUL R3, R3, R1 ; (quotient * divisor)
        SUB R15, R0, R3
        RET
        

