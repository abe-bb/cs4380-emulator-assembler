NUM_1    .INT #-1
NUM_2    .INT

    LDB R1, NUM_1
    LDA R4, NUM_2
    ILDR R2, R1
    ISTR R2, R4
    ILDR R3, R4
    TRP #1
    TRP #0