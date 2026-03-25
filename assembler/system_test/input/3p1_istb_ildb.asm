           ; alignment
           MOVI R8, #97
           MOVI R2, #16
           ISTB R8, R2
           ILDB R3, R2
SKIP       TRP #1
JMP_HERE   TRP #0 ;EXIT
