           MOVI R8, #-1
INFINITE   BGT R8, INFINITE ; R8 is -1, so this should not get stuck 
           MOVI R8, #1
           BGT r8, JMP_HERE ; R8 is 1, so this should jump to the exit
SKIP       TRP #98
JMP_HERE   TRP #0 ;EXIT
