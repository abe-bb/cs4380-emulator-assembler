           ; alignment
           MOVI R8, #-12345678
           MOVI R2, #8
           ISTR R8, R2
           ILDR R3, R2
SKIP       TRP #1
JMP_HERE   TRP #0 ;EXIT
