NUM     .INT #2
    TRP #2          ; Enter an int between 1-3 to do different comparisons
                    ; 1 for <, 2 for =, and 3 for >
    LDR R1, NUM
    CMP R2, R3, R1
    BLT R2, ONE
    BRZ R2, TWO
    BGT R2, THREE
    MOVI R3, 'f'    ; f indicates a failure
    TRP #3
    TRP #0
ONE MOVI R3, '1'
    TRP #3
    TRP #0
TWO MOVI R3, '2'
    TRP #3
    TRP #0
THREE MOVI R3, '3'
    TRP #3
    TRP #0
