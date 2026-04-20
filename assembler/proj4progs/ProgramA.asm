PROMPT  .STR "Please enter the Fibonacci term you would like computed: "
TERM    .STR "Term "
RESULT  .STR " in the Fibonacci sequence is: "

NEWLINE .BTS #0
ALT_NL  .BTS #0
NL      .BYT '\n' 

; Prompts user for input, and stores the resulting number in R14
ENTRY   LDA R3, PROMPT
        TRP #5
        TRP #2
        MOV R14, R3
        MOV R13, R3

; initialize fibonacci sequence
; R5 is term 2 R6 is term 1
        MOVI R6, #0
        MOVI R5, #1

; check if the user requested the first term
        SUBI R14, R14, #2 
        MOV R0, R6
        BLT R14, DONE

; check if the user requested the second term
        SUBI R14, R14, #1 
        MOV R0, R5
        BLT R14, DONE

; Fibonacci calculation loop
LOOP    SUBI R14, R14, #1
        ADD R4, R5, R6
        MOV R6, R5
        MOV R5, R4
        MOV R0, R4
        ; goto DONE if we found the correct term
        BLT R14, DONE
        ; otherwise jump to start of loop
        JMP LOOP


; Output results and exit
DONE    LDA R3, TERM 
        TRP #5 ; print "Term "
        MOV R3, R13
        TRP #1 ; print term number
        LDA R3, RESULT
        TRP #5 ; print " in the Fibonacci sequence is: "
        MOV R3, R0
        TRP #1 ; print the calcualted term
        LDB R3, NEWLINE
        TRP #3 ; print "\n"
        
        TRP #0
