; R15 is return value register
; R0-2 are parameter registers (additional parameters
;   must be placed on the stack)
; 
; R0-6 are caller's responsibility to save
; R7-14  and FP are callee's responsability

PROMPT  .STR "Welcome to the Prime Number Generator.\n\nThis program searches for and displays the first 20 prime numbers greater than or equal to a user provided lower bound.\n\nPlease enter a lower bound: "
OUT1    .STR "The first 20 prime numbers greater than or equal to "
OUT2    .STR " are:\n"
NL      .STR "\n"
; storage for the 20 primes
PRIMES  .BTS #80



        LDA R3, PROMPT
        TRP #5 ; prompt for input
        TRP #2 ; get input
        MOV R14, R3 ; store lower bound in R14


;       Set up loop variables
        SUBI R7, R14, #1 ; R7 is i (current number to check)
        MOVI R8, #0 ; R8 is the prime index 

LOOP    ADDI R7, R7, #1
        MOV R0, R7
        CALL isPrime
        BRZ R15, LOOP_T ; not prime, so go to loop test
        ; number is prime
        LDA R3, PRIMES  ; load primes storage location
        MULI R4, R8, #4 ; calculate current offset
        ADD R4, R4, R3  ; calcualte current prime address
        ISTR  R7, R4     ; store the prime at the current prime address
        ADDI R8, R8, #1 ; increment prime index

        ; loop test
        ; if (prime_index - 20 != 0) then loop
LOOP_T  SUBI R3, R8, #20
        BNZ R3, LOOP

        ; All 20 primes have been found and stored in PRIMES at this point
        LDA R3, OUT1 ; |
        TRP #5       ; |
        MOV R3, R14  ; | Print output message
        TRP #1       ; |
        LDA R3, OUT2 ; | 
        TRP #5 

        ; set up registers to loop through stored primes
        LDA R0, PRIMES ; primes array
        MOVI R1, #0    ; current index

LOOP_P  MULI R2, R1, #4 ; calculate array offset
        ADD R2, R2, R0  ; calcualte current prime address
        ILDR R3, R2     ; load prime
        TRP #1          ; print prime
        LDA R3, NL      ; load newline
        TRP #5          ; print newline
        ADDI R1, R1, #1 ; increment index
        SUBI R3, R1, #20
        BNZ R3, LOOP_P  ; loop 20 times
        
        TRP #0 ; exit programe


; mod(dividend, divisor) -> modulus
mod     SDIV R3, R0, R1 ; quotient
        MUL R3, R3, R1 ; (quotient * divisor)
        SUB R15, R0, R3
        RET ; return  from mod()

; isPrime(num) -> bool
isPrime PSHR R8 
        PSHR R9 ; store the registers isPrime will use

        MOV R8, R0      ; number to check
        SUBI R9, R0, #2 ; current divisor

;       Test if number is 0 or 1 and return false if so 
        MOVI R15, #0
        BLT R9, isPrimeRet

;       Test if number is equal to 2 and return true if so
        MOVI R15, #1
        BRZ R9, isPrimeRet

;       Test if number is equal to 3 and return true if so
        SUBI R3, R0, #3
        MOVI R15, #1
        BRZ R3, isPrimeRet

;       Test if number is even and return false if so
        MOVI R1, #2
        CALL mod 
        BNZ R15, PRIME_L
        MOVI R15, #0
        JMP isPrimeRet 

;       loop checking divisability
PRIME_L MOV R0, R8
        MOV R1, R9
        CALL mod
        BRZ R15, isPrimeRet ; check if num is divisible by another number > 1
        SUBI R9, R9, #2
        SUBI R3, R9, #1
        BNZ R3, PRIME_L ; loop if divisor > 1

        MOVI R15, #1 ; if we make it through the loop then the number is prime
        JMP isPrimeRet ; return true

; return from isPrime
isPrimeRet POPR R9
           POPR R8
           RET
