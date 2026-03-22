           MOVI R8, #0
INFINITE   BLT R8, INFINITE ; R8 is 0, so this should not get stuck 
           MOVI R8, #-1
           BLT r8, JMP_HERE ; R8 is -1, so this should jump to the exit
SKIP       TRP #98
JMP_HERE   TRP #0 ;EXIT
