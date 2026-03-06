; comment line to make the lines match up :)
F1r$T  .iNt
1FIRST .INT #-2147483648 ; this is a comment
       .INT #2147483647
0ZERO  .INT #0           ; hello world
BYT    .BYT              ; unspecificed BYT
       .bYt #255
0LF    .BYT '\n'
1TB    .BYT '\t'
       .BYT '\\'
       .BYT '\''
       .BYT '\"'
       .BYT '\r'
       .BYT '\b'
       .BYT 'A'
       .BYT 'z'
       .bYt #0
  LdR SL, F1r$T
  LDr SB, 1FIRST
  lDR SP, '\b'
  LDR FP, #16
