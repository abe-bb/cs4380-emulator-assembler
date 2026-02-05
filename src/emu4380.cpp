#include "../include/emu4380.h"
#include <algorithm>
#include <iostream>
#include <vector>

unsigned int MEM_SIZE = 0b1 << 17;

unsigned int reg_file[22] = {0};
unsigned char* prog_mem = 0;
unsigned int cntrl_regs[5] = {0};

bool validate_address(unsigned int address, unsigned int size = 4) {
  return address <= MEM_SIZE - size;
}

bool jmp() {
  // can't jump to the last 7 bytes of program memory
  if (!validate_address(cntrl_regs[IMMEDIATE], 8)) {
    return false;
  }

  reg_file[PC] = cntrl_regs[IMMEDIATE];
  return true;
}

bool mov() {
  auto r_src = cntrl_regs[OPERAND_2];
  auto r_dest = cntrl_regs[OPERAND_1];

  reg_file[r_dest] = reg_file[r_src];  
  return true;
}

bool movi() {
  auto r_dest = cntrl_regs[OPERAND_1];

  reg_file[r_dest] = cntrl_regs[IMMEDIATE];
  return true;
}

bool lda() {
  auto r_dest = cntrl_regs[OPERAND_1];
  auto address = cntrl_regs[IMMEDIATE];

  if (!validate_address(address)) {
    return false;
  }

  reg_file[r_dest] = *(unsigned int*)(prog_mem + address);
  return true;
}

bool str() {
  auto r_src = cntrl_regs[OPERAND_1];
  auto address = cntrl_regs[IMMEDIATE];

  if (!validate_address(address)) {
    return false;
  }

  *(unsigned int*)(prog_mem + address) = reg_file[r_src];
  return true;
}

bool ldr() {
  auto r_dest = cntrl_regs[OPERAND_1];
  auto address = cntrl_regs[IMMEDIATE];

  if (!validate_address(address)) {
    return false;
  }

  reg_file[r_dest] = *(unsigned int*)(prog_mem + address);
  return true;
}

bool stb() {
  auto r_src = cntrl_regs[OPERAND_1];
  auto address = cntrl_regs[IMMEDIATE];

  if (!validate_address(address, 1)) {
    return false;
  }

  prog_mem[address] = (unsigned char)(reg_file[r_src] & 0x000000FF);
  return true;
}

bool ldb() {
  auto r_dest = cntrl_regs[OPERAND_1];
  auto address = cntrl_regs[IMMEDIATE];

  if (!validate_address(address, 1)) {
    return false;
  }

  reg_file[r_dest] = prog_mem[address];
  return true;
}

bool add() {
  auto r_dest = cntrl_regs[OPERAND_1];
  auto r_src1 = cntrl_regs[OPERAND_2];
  auto r_src2 = cntrl_regs[OPERAND_3];

  reg_file[r_dest] = reg_file[r_src1] + reg_file[r_src2];
  return true;
}

bool addi() {
  auto r_dest = cntrl_regs[OPERAND_1];
  auto r_src1 = cntrl_regs[OPERAND_2];
  auto immed = cntrl_regs[IMMEDIATE];

  reg_file[r_dest] = reg_file[r_src1] + immed;
  return true;
}

bool sub() {
  auto r_dest = cntrl_regs[OPERAND_1];
  auto r_src1 = cntrl_regs[OPERAND_2];
  auto r_src2 = cntrl_regs[OPERAND_3];

  reg_file[r_dest] = reg_file[r_src1] - reg_file[r_src2];
  return true;
}

bool subi() {
  auto r_dest = cntrl_regs[OPERAND_1];
  auto r_src1 = cntrl_regs[OPERAND_2];
  auto immed = cntrl_regs[IMMEDIATE];

  reg_file[r_dest] = reg_file[r_src1] - immed;
  return true;
}

bool mul() {
  auto r_dest = cntrl_regs[OPERAND_1];
  auto r_src1 = cntrl_regs[OPERAND_2];
  auto r_src2 = cntrl_regs[OPERAND_3];

  reg_file[r_dest] = reg_file[r_src1] * reg_file[r_src2];
  return true;
}

bool muli() {
  auto r_dest = cntrl_regs[OPERAND_1];
  auto r_src1 = cntrl_regs[OPERAND_2];
  auto immed = cntrl_regs[IMMEDIATE];

  reg_file[r_dest] = reg_file[r_src1] * immed;
  return true;
}

bool div() {
  auto r_dest = cntrl_regs[OPERAND_1];
  auto r_src1 = cntrl_regs[OPERAND_2];
  auto r_src2 = cntrl_regs[OPERAND_3];

  // can't divide by zero
  if (reg_file[r_src2] == 0) {
    return false;
  }

  reg_file[r_dest] = reg_file[r_src1] / reg_file[r_src2];
  return true;
}

bool sdiv() {
  auto r_dest = cntrl_regs[OPERAND_1];
  auto r_src1 = cntrl_regs[OPERAND_2];
  auto r_src2 = cntrl_regs[OPERAND_3];

  // can't divide by zero
  if (reg_file[r_src2] == 0) {
    return false;
  }

  reg_file[r_dest] = (unsigned int)((signed int)reg_file[r_src1] / (signed int)reg_file[r_src2]);
  return true;
}

bool divi() {
  auto r_dest = cntrl_regs[OPERAND_1];
  auto r_src1 = cntrl_regs[OPERAND_2];
  auto immed = cntrl_regs[IMMEDIATE];

  if (immed == 0) {
    return false;
  }

  reg_file[r_dest] = reg_file[r_src1] / immed;
  return true;
}

bool init_mem(unsigned int size) {
  prog_mem = new unsigned char[size];
  MEM_SIZE = size;
  return true;
}

// bool fetch(); // Retrieves the bytes for the current instruction and places
// them in the appropriate cntrl_regs. Also increments the PC to point to the
// next instruction. If an invalid fetch address (i.e. out of bounds) is
// encountered by this funcLon it shall return false. Otherwise it shall return
// true
bool fetch() {
  // check that PC is within program memory
  if (reg_file[PC] > MEM_SIZE - 8) {
    return false;
  }

  auto load_addr = reg_file[PC];

  // load memory into control registers
  cntrl_regs[OPERATION] = prog_mem[load_addr];
  cntrl_regs[OPERAND_1] = prog_mem[load_addr + 1];
  cntrl_regs[OPERAND_2] = prog_mem[load_addr + 2];
  cntrl_regs[OPERAND_3] = prog_mem[load_addr + 3];
  // cast to unsigned int pointer and dereference (assumes little endian environment)
  cntrl_regs[IMMEDIATE] = *(unsigned int*)(prog_mem + load_addr + 4);
                          
  // increment PC and return true
  reg_file[PC] += 8;
    return true;
}

// This function shall verify that the specified operation (or
// TRP) and operands as specified in the cntrl_regs are valid (i.e. a “known”
// instruction with legal operands). For example: a MOV instruction operates
// on state registers, and there are a limited number of these; a MOV
// instruction with an RD value of 55 would clearly be a malformed
// instruction.
bool decode() {
  // validate operation (1, 7-13, 18-26, 31)
  auto op = cntrl_regs[OPERATION];
  if (!(op == 1 ||
     (op >= 7 && op <= 13) ||
     (op >= 18 && op <= 26) ||
      op == 31)) {
    return false;
  }

  // read operands from control registers
  auto op1 = cntrl_regs[OPERAND_1];
  auto op2 = cntrl_regs[OPERAND_2];
  auto op3 = cntrl_regs[OPERAND_3];
  
  // operation doesn't care about any operands, so return true
  auto begin = operations_0operand_3dc.begin();
  auto end = operations_0operand_3dc.end();
  if (std::find(begin, end, op) != end) {

    return true;
  }
  // operation cares about operand 1, so ignore operand 2 and 3
  begin = operations_1operand_2dc.begin();
  end = operations_1operand_2dc.end();
  if (std::find(begin, end, op) != end) {
    op2 = R0;
    op3 = R0;
  }
  // operation cares about operand 1 and 2, so ignore operand 3
  begin = operations_2operand_1dc.begin();
  end = operations_2operand_1dc.end();
  if (std::find(begin, end, op) != end) {
    op3 = R0;
  }
  // all 3 operations are cared about, so don't ignore any oeprands

  // valdiate operands and return
  return op1 <= 21 && op2 <= 21 && op3 <= 21;
}

bool execute() {
  switch(cntrl_regs[OPERATION]) {
    case JMP:
      return jmp();
    case MOV:
      return mov();
    case MOVI:
      return movi();
    case LDA:
      return lda();
    case STR:
      return str();
    case LDR:
      return ldr();
    case STB:
      return stb();
    case LDB:
      return ldb();
    case ADD:
      return add();
    case ADDI:
      return addi();
    case SUB:
      return sub();
    case SUBI:
      return subi();
    case MUL:
      return mul();
    case MULI:
      return muli();
    case DIV:
      return div();
    case SDIV:
      return sdiv();
    case DIVI:
      return divi();        
    default:
      std::cerr << "execute() called with invalid operation!";
      throw "Can't handle invalid operation!";
      
  }
  return false;
}

// convenience categorization of operations
std::vector<unsigned int> operations_0operand_3dc = {1, 31};
std::vector<unsigned int> operations_1operand_2dc = {8, 9, 10, 11, 12, 13};
std::vector<unsigned int> operations_2operand_1dc = {7, 19, 21, 23, 26};
std::vector<unsigned int> operations_3operand_0dc = {18, 20, 22, 24, 25};

