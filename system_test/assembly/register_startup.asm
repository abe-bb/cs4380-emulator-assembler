SEP     .STR ", "

; Write SB to stdout 
            MOV R3, SB
            TRP #1
            LDA R3, SEP
            TRP #5

; Write SP to stdout 
            MOV R3, SP
            TRP #1
            LDA R3, SEP
            TRP #5

; Write SP to stdout 
            MOV R3, SL
            TRP #1

; Exit
            TRP #0

        
        
