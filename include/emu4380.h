#pragma once

#include <vector>
extern unsigned int MEM_SIZE;

extern unsigned int reg_file[22];
extern unsigned char* prog_mem;
extern unsigned int cntrl_regs[5];

enum RegNames { R0=0, R1, R2, R3, R4, R5, R6, R7, R8, R9, R10, R11, R12, R13, R14, R15, PC, SL, SB, SP, FP, HP };

enum CntrlRegNames{ OPERATION, OPERAND_1, OPERAND_2, OPERAND_3, IMMEDIATE };

enum Operations{JMP=1, MOV=7, MOVI, LDA, STR, LDR, STB, LDB, ADD = 18, ADDI, SUB, SUBI, MUL, MULI, DIV, SDIV, DIVI, TRP=31};

bool init_mem(unsigned int size);

bool fetch();
bool decode();
bool execute();


// convenience categorization of operations
extern std::vector<unsigned int> operations_0operand_3dc;
extern std::vector<unsigned int> operations_1operand_2dc;
extern std::vector<unsigned int> operations_2operand_1dc;
extern std::vector<unsigned int> operations_3operand_0dc;



