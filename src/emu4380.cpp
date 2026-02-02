#include "../include/emu4380.h"

unsigned int MEM_SIZE = 0b1 << 17;

unsigned int reg_file[22] = {0};
unsigned char* prog_mem = 0;
unsigned int cntrl_regs[5] = {0};

bool init_mem(unsigned int size) {
  prog_mem = new unsigned char[size];
  MEM_SIZE = size;
  return true;
}

// bool fetch(); // Retrieves the bytes for the current instrucLon and places
// them in the appropriate cntrl_regs. Also increments the PC to point to the
// next instrucLon. If an invalid fetch address (i.e. out of bounds) is
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
  cntrl_regs[IMMEDIATE] = prog_mem[load_addr + 4] |
                          prog_mem[load_addr + 5] << 8 |
                          prog_mem[load_addr + 6] << 16 |
                          prog_mem[load_addr + 7] << 24;
                          

  // increment PC and return true
  reg_file[PC] += 8;
    return true;
}

// This funcLon shall verify that the specified operation (or
// TRP) and operands as specified in the cntrl_regs are valid (i.e. a “known”
// instruction with legal operands). For example: a MOV instruction operates
// on state registers, and there are a limited number of these; a MOV
// instruction with an RD value of 55 would clearly be a malformed
// instruction.
bool decode() {
  return false;
}

bool execute() {
  return false;
}
