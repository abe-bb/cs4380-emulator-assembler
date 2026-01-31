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

bool fetch() {
  return false;
}

bool decode() {
  return false;
}

bool execute() {
  return false;
}
