lbl .INT #128
    ALCI R6, #64
    ALLC R7, lbl
    LDR R9, lbl
    IALLC R8, R9
    TRP #0 ;EXIT
