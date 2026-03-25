NUM_1       .INT #-1

    LDR R1, NUM_1
    CMPI R3,R1, #-1
    BRZ R3, SUCCESS
    MOVI R3, 'f'
    TRP #3
    TRP #0
SUCCESS MOVI R3, 's'
    TRP #3
    TRP #0