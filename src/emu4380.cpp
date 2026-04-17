#include "../include/emu4380.h"
#include "../include/cache.h"

#include <algorithm>
#include <cstdio>
#include <iostream>
#include <stdexcept>
#include <vector>

unsigned int MEM_SIZE = 0b1 << 17;

unsigned int reg_file[22] = {0};
unsigned char* prog_mem = 0;
unsigned int cntrl_regs[5] = {0};

unsigned int mem_cycle_cntr = 0;

Cache* cache = new Cache(nullptr);

unsigned int cache_type = 0;


PostOpFlag flag = NOTHING;

const unsigned int BLOCK_SIZE = 16;
const unsigned int CACHE_LINES = 64;

bool validate_address(unsigned int address, unsigned int size = 4) {
  return address <= MEM_SIZE - size;
}

unsigned char readByte(unsigned int address) {
  unsigned char byte;
  mem_cycle_cntr += cache->readByte(address, byte);
  return byte;
}

unsigned int readWord(unsigned int address) {
  unsigned int word;
  mem_cycle_cntr += cache->readWord(address, word);
  return word;
}

void writeByte(unsigned int address, unsigned char byte) {
  mem_cycle_cntr += cache->writeByte(address, byte);
}

void writeWord(unsigned int address, unsigned int word) {
  mem_cycle_cntr += cache->writeWord(address, word);
}

bool jmp() {
  // can't jump to the last 7 bytes of program memory (or beyond)
  if (!validate_address(cntrl_regs[IMMEDIATE], 8)) {
    return false;
  }

  reg_file[PC] = cntrl_regs[IMMEDIATE];
  return true;
}


bool jmr() {
  auto dest = cntrl_regs[OPERAND_1];

  // can't jump to the last 7 bytes of program memory (or beyond)
  if (!validate_address(reg_file[dest], 8)) {
    return false;
  }

  reg_file[PC] = reg_file[dest];
  return true;
}

bool bnz() {
  auto condition = cntrl_regs[OPERAND_1];
  auto address = cntrl_regs[IMMEDIATE];

  // can't jump to the last 7 bytes of program memory (or beyond)
  if (!validate_address(address, 8)) {
    return false;
  }

  // check condition
  if ((int)reg_file[condition] != 0) {
    reg_file[PC] = address;
  }
  return true;
}

bool bgt() {
  auto condition = cntrl_regs[OPERAND_1];
  auto address = cntrl_regs[IMMEDIATE];

  // can't jump to the last 7 bytes of program memory (or beyond)
  if (!validate_address(address, 8)) {
    return false;
  }

  // check condition
  if ((int)reg_file[condition] > 0) {
    reg_file[PC] = address;
  }
  return true;
}

bool blt() {
  auto condition = cntrl_regs[OPERAND_1];
  auto address = cntrl_regs[IMMEDIATE];

  // can't jump to the last 7 bytes of program memory (or beyond)
  if (!validate_address(address, 8)) {
    return false;
  }

  // check condition
  if ((int)reg_file[condition] < 0) {
    reg_file[PC] = address;
  }
  return true;
}

bool brz() {
  auto condition = cntrl_regs[OPERAND_1];
  auto address = cntrl_regs[IMMEDIATE];

  // can't jump to the last 7 bytes of program memory (or beyond)
  if (!validate_address(address, 8)) {
    return false;
  }

  // check condition
  if ((int)reg_file[condition] == 0) {
    reg_file[PC] = address;
  }
  return true;
}

bool istr() {
  auto r_src = cntrl_regs[OPERAND_1];
  auto r_addr = cntrl_regs[OPERAND_2];

  if (!validate_address(reg_file[r_addr])) {
    return false;
  }

  writeWord(reg_file[r_addr], reg_file[r_src]);
  return true;
}

bool ildr() {
  auto r_dest = cntrl_regs[OPERAND_1];
  auto r_addr = cntrl_regs[OPERAND_2];

  if (!validate_address(reg_file[r_addr])) {
    return false;
  }

  reg_file[r_dest] = readWord(reg_file[r_addr]);
  return true;
}

bool istb() {
  auto r_src = cntrl_regs[OPERAND_1];
  auto r_addr = cntrl_regs[OPERAND_2];

  if (!validate_address(reg_file[r_addr], 1)) {
    return false;
  }

  writeByte(reg_file[r_addr], reg_file[r_src] & 0xFF);
  return true;
}

bool ildb() {
  auto r_dest = cntrl_regs[OPERAND_1];
  auto r_addr = cntrl_regs[OPERAND_2];

  if (!validate_address(reg_file[r_addr], 1)) {
    return false;
  }

  reg_file[r_dest] = readByte(reg_file[r_addr]);
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

  reg_file[r_dest] = address;
  return true;
}

bool str() {
  auto r_src = cntrl_regs[OPERAND_1];
  auto address = cntrl_regs[IMMEDIATE];

  if (!validate_address(address)) {
    return false;
  }

  writeWord(address, reg_file[r_src]);
  return true;
}

bool ldr() {
  auto r_dest = cntrl_regs[OPERAND_1];
  auto address = cntrl_regs[IMMEDIATE];

  if (!validate_address(address)) {
    return false;
  }

  reg_file[r_dest] = readWord(address);
  return true;
}

bool stb() {
  auto r_src = cntrl_regs[OPERAND_1];
  auto address = cntrl_regs[IMMEDIATE];

  if (!validate_address(address, 1)) {
    return false;
  }

  writeByte(address, reg_file[r_src] & 0xFF);
  return true;
}

bool ldb() {
  auto r_dest = cntrl_regs[OPERAND_1];
  auto address = cntrl_regs[IMMEDIATE];

  if (!validate_address(address, 1)) {
    return false;
  }

  reg_file[r_dest] = readByte(address);
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

  // can't divide by zero
  if (immed == 0) {
    return false;
  }

  // signed division
  reg_file[r_dest] = (unsigned int)((signed int)reg_file[r_src1] / (signed int)immed);
  return true;
}

bool and_inst() {
  auto r_dest = cntrl_regs[OPERAND_1];
  auto r_src1 = cntrl_regs[OPERAND_2];
  auto r_src2 = cntrl_regs[OPERAND_3];

  reg_file[r_dest] = ((bool)reg_file[r_src1]) && ((bool)reg_file[r_src2]);
  
  return true;
}

bool or_inst() {
  auto r_dest = cntrl_regs[OPERAND_1];
  auto r_src1 = cntrl_regs[OPERAND_2];
  auto r_src2 = cntrl_regs[OPERAND_3];

  reg_file[r_dest] = reg_file[r_src1] || reg_file[r_src2];

  return true;
}

bool cmp() {
  auto r_dest = cntrl_regs[OPERAND_1];
  signed int s1 = reg_file[cntrl_regs[OPERAND_2]];
  signed int s2 = reg_file[cntrl_regs[OPERAND_3]];

  if (s1 == s2) {
    reg_file[r_dest] = 0;
  }
  else if (s1 > s2) {
    reg_file[r_dest] = 1;
  }
  else {
    reg_file[r_dest] = -1;
  }
  return true;
}

bool cmpi() {
  auto r_dest = cntrl_regs[OPERAND_1];
  signed int s1 = reg_file[cntrl_regs[OPERAND_2]];
  signed int s2 = cntrl_regs[IMMEDIATE];

  if (s1 == s2) {
    reg_file[r_dest] = 0;
  }
  else if (s1 > s2) {
    reg_file[r_dest] = 1;
  }
  else {
    reg_file[r_dest] = -1;
  }
  return true;
}

bool trp0() {
  flag = TERMINATE;
  return true;
}

bool trp1() {
  std::cout << (signed int) reg_file[R3];
  return true;
}

bool trp2() {
  std::string input;
  std::cin >> input;

  int potential_int;
  if (!parse_int(input, potential_int)) {
    std::cout << "\"" << input << "\" is either not within range or not an integer.\n" << std::flush; 
    exit(5);

  }

  reg_file[R3] = potential_int;
  return true;
}

bool trp3() {
  std::cout << (char)reg_file[R3];
  return true;
}

bool trp4() {
  char input;
  // input  = getchar();
  std::cin >> input;
  reg_file[R3] = input;
  return true;
}

bool trp5() {
  auto addr = reg_file[R3];

  if (!validate_address(addr, 2)) {
    return false;
  }

  unsigned char max_str_len = readByte(addr);

  for (unsigned int i = 1; i <= max_str_len;  i++) {
    char byte = readByte(addr + i);
    // null termination 
    if (byte == 0) {
      break;
    }

    std::cout << byte;
  }

  return true;
}

bool trp6() {
  return false;
}

std::string sp_reg_names[] = {"PC", "SL", "SB", "SP", "FP", "HP"};
bool trp98() {
  for (int i = 0; i < 22; i++) {
    if (i < 16) {
      std::cout << "R" << i;
    }
    else {
      std::cout << sp_reg_names[i - 16];
    }

    std::cout << "\t" << reg_file[i] << "\n";
  }

  return true;
}

bool trp() {
  auto immed = cntrl_regs[IMMEDIATE];

  // validate immediate
  if (!(immed <= 6 || immed == 98)) {
    return false;
  }

  switch (immed) {
    case 0:
      return trp0();
    case 1:
      return trp1();
    case 2:
      return trp2();
    case 3:
      return trp3();
    case 4:
      return trp4();
    case 5:
      return trp5();
    case 6:
      return trp6();
    case 98:
      return trp98();
    default:
      std::cout << "TRP error! Invalid immediate value not detected.";
      throw "Can't handle invalid trp code not detected!";
  }
}

bool alci() {
  auto r_dest = cntrl_regs[OPERAND_1];
  auto bytes = cntrl_regs[IMMEDIATE];

  // check that the heap would still fit in memory
  if (reg_file[HP] + bytes > MEM_SIZE) {
    return false;
  }

  reg_file[r_dest] = reg_file[HP];
  reg_file[HP] += bytes;
  return true;
}

bool allc() {
  auto r_dest = cntrl_regs[OPERAND_1];
  auto address = cntrl_regs[IMMEDIATE];

  // check that the address is in range
  if (!validate_address(address)) {
    return false;
  }

  unsigned int bytes = *(unsigned int*)(prog_mem + address);

  // check that the heap would still fit in memory
  if (reg_file[HP] + bytes > MEM_SIZE) {
    return false;
  }
  
  reg_file[r_dest] = reg_file[HP];
  reg_file[HP] += bytes;

  return true;
}

bool iallc() {
  auto r_dest = cntrl_regs[OPERAND_1];
  auto r_addr = cntrl_regs[OPERAND_2];

  unsigned int address = reg_file[r_addr];

  // check that the address is in range
  if (!validate_address(address)) {
    return false;
  }

  unsigned int bytes = *(unsigned int*)(prog_mem + address);
  // check that the heap would still fit in memory
  if (reg_file[HP] + bytes > MEM_SIZE) {
    return false;
  }
  
  reg_file[r_dest] = reg_file[HP];
  reg_file[HP] += bytes;

  return true;
}

bool pshr() {
  auto r_src = cntrl_regs[OPERAND_1];

  // check that SP will be within bounds
  if (reg_file[SP] - 4 < reg_file[SL]) {
    return false;
  }

  // write the word to the stack
  reg_file[SP] -= 4;
  writeWord(reg_file[SP], reg_file[r_src]);
  
  return true;
}

bool pshb() {
  auto r_src = cntrl_regs[OPERAND_1];

  // check that SP will be within bounds
  if (reg_file[SP] - 1 < reg_file[SL]) {
    return false;
  }

  // write the byte to the stack
  reg_file[SP] -= 1;
  writeByte(reg_file[SP], reg_file[r_src]);
  
  return true;
}

bool popr() {
  auto r_dest = cntrl_regs[OPERAND_1];

  // check that SP will be within bounds
  if (reg_file[SP] + 4 > reg_file[SB]) {
    return false;
  }

  // read the word from the stack
  reg_file[r_dest] = readWord(reg_file[SP]);
  reg_file[SP] += 4;
  
  return true;
}

bool popb() {
  auto r_dest = cntrl_regs[OPERAND_1];

  // check that SP will be within bounds
  if (reg_file[SP] + 1 > reg_file[SB]) {
    return false;
  }

  // read the byte from the stack
  reg_file[r_dest] = readByte(reg_file[SP]);
  reg_file[SP] += 1;
  
  return true;
}

bool call() {
  auto addr = cntrl_regs[IMMEDIATE];

  if (!validate_address(addr)) {
    return false;
  }
    
  // check that SP will be within bounds
  if (reg_file[SP] - 4 < reg_file[SL]) {
    return false;
  }

  // write PC to the stack
  reg_file[SP] -= 4;
  writeWord(reg_file[SP], reg_file[PC]);

  // set PC to address
  reg_file[PC] = addr;
  
  return true;  
}

bool ret() {
  // check that SP will be within bounds
  if (reg_file[SP] + 4 > reg_file[SB]) {
    return false;
  }

  // read the address from the stack into PC
  reg_file[PC] = readWord(reg_file[SP]);
  reg_file[SP] += 4;

  return true;
}

bool init_mem(unsigned int size) {
  prog_mem = new unsigned char[size];
  MEM_SIZE = size;
  return true;
}

// must be called after init_mem and before any memory access functions
// are called
void init_cache(unsigned int cacheType) {
  delete cache;

  cache_type = cacheType;

  switch (cacheType) {
    case 0:
        cache = new Cache(prog_mem);
        break;
    case 1:
        cache = new NWayCache(prog_mem, BLOCK_SIZE, CACHE_LINES, 1);
        break;
    case 2:
        cache = new NWayCache(prog_mem, BLOCK_SIZE, CACHE_LINES, CACHE_LINES);
        break;
    case 3:
        cache = new NWayCache(prog_mem, BLOCK_SIZE, CACHE_LINES, 2);
        break;
    default:
        throw "init_cache called with unknown cache type!";
    }
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
  unsigned int ops = readWord(load_addr);
  cntrl_regs[OPERATION] = ops & 0xFF;
  cntrl_regs[OPERAND_1] = (ops & 0xFF00) >> 8;
  cntrl_regs[OPERAND_2] = (ops & 0xFF0000) >> 16;
  cntrl_regs[OPERAND_3] = (ops & 0xFF000000) >> 24;
  // cast to unsigned int pointer and dereference (assumes little endian environment)
  cntrl_regs[IMMEDIATE] = readWord(load_addr + 4);

  if (cache_type == 0) {
    mem_cycle_cntr -= 6;
  }
               
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
  // validate operation (1-40)
  auto op = cntrl_regs[OPERATION];
  if ((op < 1 || op > 40)) {
    return false;
  }

  // read operands from control registers
  auto op1 = cntrl_regs[OPERAND_1];
  auto op2 = cntrl_regs[OPERAND_2];
  auto op3 = cntrl_regs[OPERAND_3];

  // validate trp immediate value
  if (op == 31) {
    unsigned int imm = cntrl_regs[IMMEDIATE];

    if (!(imm <= 6 || imm == 98)) {
      return false;
    }
  }
  
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
  // all 3 operations are cared about, so don't ignore any operands

  // valdiate operands and return
  return op1 <= 21 && op2 <= 21 && op3 <= 21;
}

bool execute() {
  switch(cntrl_regs[OPERATION]) {
    case JMP:
      return jmp();
    case JMR:
      return jmr();
    case BNZ:
      return bnz();
    case BGT:
      return bgt();
    case BLT:
      return blt();
    case BRZ:
      return brz();
    case ISTR:
      return istr();
    case ILDR:
      return ildr();
    case ISTB:
      return istb();
    case ILDB:
      return ildb();
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
    case AND:
      return and_inst();
    case OR:
      return or_inst();
    case CMP:
      return cmp();
    case CMPI:
      return cmpi();
    case TRP:
      return trp();
    case ALCI:
      return alci();
    case ALLC:
      return allc();
    case IALLC:
      return iallc();
    case PSHR:
      return pshr();
    case PSHB:
      return pshb();
    case POPR:
      return popr();
    case POPB:
      return popb();
    case CALL:
      return call();
    case RET:
      return ret();
    default:
      std::cout << "execute() called with invalid operation!";
      throw std::runtime_error("Can't handle invalid operation!");
      
  }
  return false;
}

// convenience categorization of operations
std::vector<unsigned int> operations_0operand_3dc = {1, 31, 39, 40};
std::vector<unsigned int> operations_1operand_2dc = {8, 9, 10, 11, 12, 13, JMR, BNZ, BGT, BLT, BRZ, 35, 36, 37, 38, 32, 33};
std::vector<unsigned int> operations_2operand_1dc = {7, 19, 21, 23, 26, ISTR, ILDR, ISTB, ILDB, CMPI, 34};
std::vector<unsigned int> operations_3operand_0dc = {18, 20, 22, 24, 25, CMP, 27, 28};

bool parse_unsigned_int(std::string input, unsigned int &output) {
  try {
    size_t chars_processed = 0;
    // parse input 
    unsigned long base10 = std::stoul(input, &chars_processed);
    if (base10 > 4294967295) {
      throw std::out_of_range("");
    }

    output = base10;
    return true;
  }
  // handle stoi excpetions
  catch (std::invalid_argument e) {
    return false;
  }
  catch (std::out_of_range e) {
    return false;
  }
}

bool parse_int(std::string input, int &output) {
  try {
    // parse input 
    int base10 = std::stoi(input);

    output = base10;
    return true;
  }
  // handle stoi excpetions
  catch (std::invalid_argument e) {
    return false;
  }
  catch (std::out_of_range e) {
    return false;
  }
}
