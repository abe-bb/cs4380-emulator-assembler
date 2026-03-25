CHAR_1    .BYT 'S'
CHAR_2    .BYT

    LDB R1, CHAR_1
    LDA R4, CHAR_2
    ILDB R2, R1
    ISTB R2, R4
    ILDB R3, R4
    TRP #3
    TRP #0