#include "../include/emu4380.h"

#include <algorithm>
#include <cstdio>
#include <iostream>
#include <stdexcept>
#include <vector>
#include <climits>

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
  auto addr = reg_file[R3];

  std::string input;
  getline(std::cin, input);

  auto total_len = input.size() + 2;
  if (!validate_address(addr, total_len) && input.size() <= 255) {
    return false;
  }

  // write string size
  writeByte(addr, input.size());

  // write string
  for (auto i = 0; i < input.size(); i++) {
    writeByte(addr + i + 1, input[i]);
  }

  // write null terminator
  writeByte(addr + input.size() + 1, 0);
  return true;
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

// Cache base class
Cache::Cache(unsigned char* memory) {
  this->memory = memory;
}

Cache::~Cache() {}

// dummy classes read straight from program memory
unsigned int Cache::readByte(unsigned int address, unsigned char& outByte) {
  outByte = prog_mem[address];
  return 8;
}

unsigned int Cache::writeByte(unsigned int address, unsigned char byte) {
  prog_mem[address] = byte;
  return 8;
}

unsigned int Cache::readWord(unsigned int address, unsigned int& outWord) {
   outWord = *(unsigned int*)(prog_mem + address);
   return 8;
}

unsigned int Cache::writeWord(unsigned int address, unsigned int word) {
  *(unsigned int*)(prog_mem + address) = word;
  return 8;
}


// CacheTime
CacheTime::CacheTime() : hits(1), words_stored(0), words_loaded(0) {}

unsigned int CacheTime::calculate_timing() {
  unsigned int mem_timing = hits;

  if (words_stored != 0) {
    mem_timing += 8;
    words_stored -= 1;
    mem_timing += words_stored * 2;
  }

  if (words_loaded != 0) {
    mem_timing += 8;
    words_loaded -= 1;
    mem_timing += words_loaded * 2;
  }

  return mem_timing;
}

void CacheTime::add(CacheTime other) {
  hits += other.hits;
  words_stored += other.words_stored;
  words_loaded += other.words_loaded;
}


// NWayCache
NWayCache::NWayCache(unsigned char* prog_mem, unsigned int block_size,
                     unsigned int cache_lines, unsigned int associativitiy) : Cache(prog_mem) {
  // check block conditions
  if (block_size < 4 || cache_lines < 1 ||
      // check for cache_lines and block_size not being powers of 2
      (block_size & (block_size - 1)) != 0 || (cache_lines & (cache_lines - 1)) != 0 ||
      // check that cache lines is cleanly divisible by associativity
      (cache_lines % associativitiy != 0)) {

    throw std::invalid_argument("Invalid cache configuration");
  }

  this->sets = std::vector<CacheSet>();

  // create CacheSets and CacheLines
  unsigned int num_sets = cache_lines / associativitiy;
  unsigned int block_bits = 0;
  unsigned int set_bits = 0;

  unsigned int v = block_size;
  while (v >>= 1) {
    block_bits += 1;
  }

  v = num_sets;
  while (v >>= 1) {
    set_bits += 1;
  }

  unsigned int tag_bits = 32 - block_bits - set_bits;

  this->tag_bits = tag_bits;
  this->set_bits = set_bits;
  this->block_bits = block_bits;

  this->block_mask = (1 << block_bits) - 1;
  this->set_mask = ((1 << set_bits) - 1) << block_bits;
  this->tag_mask = ((1 << tag_bits) - 1) << block_bits << set_bits;

  unsigned int set_id = 0;
  for (auto i = 0; i < num_sets; i++) {
    std::vector<CacheLine> lines;
    for (auto j = 0; j < associativitiy; j++) {    
      CacheLine line = CacheLine(block_size, tag_bits, block_bits);
      lines.push_back(line);
    }
    CacheSet set = CacheSet(set_id, tag_bits, set_bits, lines, prog_mem);
    sets.push_back(set);
    set_id += 1;
  }
}

unsigned int NWayCache::readByte(unsigned int address, unsigned char& outByte) {
  auto set_id = get_set_id(address);

  for (CacheSet& set : sets) {
    if (set.get_set() != set_id) {
      continue;
    }

    unsigned int readWord = 0;
    unsigned int tag = (address & tag_mask) >> (32 - tag_bits);
    auto timing = set.readWord(tag, address & block_mask, readWord);

    outByte = readWord & 0xFF;

    return timing.calculate_timing();
  }

  throw std::runtime_error("Failed to find matching set in cache!!");
}

unsigned int NWayCache::writeByte(unsigned int address, unsigned char byte) {
  auto set_id = get_set_id(address);

  for (CacheSet& set : sets) {
    if (set.get_set() != set_id) {
      continue;
    }

    unsigned int tag = (address & tag_mask) >> (32 - tag_bits);
    auto timing = set.writeWord(tag, address & block_mask, byte, 1);

    return timing.calculate_timing();
  }

  throw std::runtime_error("Failed to find matching set in cache!!");
}

unsigned int NWayCache::readWord(unsigned int address, unsigned int& outWord) {
  // detect split block reads
  auto set_id = get_set_id(address);
  unsigned int second_addr = 0;
  unsigned int second_addr_bytes = split_blocks(address, second_addr);
  unsigned int first_addr_bytes = 4 - second_addr_bytes;

  outWord = 0;

  CacheTime first_timing = CacheTime();
  for (CacheSet& set : sets) {
   if (set.get_set() != set_id) {
     continue;
   }

   unsigned int readWord = 0;
   unsigned int tag = (address & tag_mask) >> (32 - tag_bits);
   first_timing = set.readWord(tag, address & block_mask, readWord);

   unsigned long p1_mask = 1;
   p1_mask = (p1_mask << (first_addr_bytes * 8)) - 1;
   unsigned int part1 = readWord & p1_mask;
   part1 = part1 << (second_addr_bytes * 8);
   outWord = readWord;
   break;
  }

  if (second_addr_bytes == 0) {
   return first_timing.calculate_timing();
  }

  set_id = get_set_id(second_addr);
  // read from second block
  for (CacheSet& set : sets) {
   if (set.get_set() != set_id) {
     continue;
   }

   unsigned int readWord = 0;
   unsigned int tag = (second_addr & tag_mask) >> (32 - tag_bits);
   auto second_timing = set.readWord(tag, second_addr & block_mask, readWord);

   outWord |= (readWord & (1 << (second_addr_bytes * 8)) - 1) << (first_addr_bytes * 8);

   first_timing.add(second_timing);
   return first_timing.calculate_timing();
  }

   throw std::runtime_error("Failed to find matching set in cache!!");
}

unsigned int NWayCache::writeWord(unsigned int address, unsigned int word) {
   // detect split block writes
   auto set_id = get_set_id(address);
   unsigned int second_addr = 0;
   unsigned int second_addr_bytes = split_blocks(address, second_addr);
   unsigned int first_addr_bytes = 4 - second_addr_bytes;

   CacheTime first_timing = CacheTime();
   for (CacheSet& set : sets) {
     if (set.get_set() != set_id) {
       continue;
     }

     unsigned int tag = (address & tag_mask) >> (32 - tag_bits);
     first_timing = set.writeWord(tag, address & block_mask, word, first_addr_bytes);
     break;
   }
   
   if (second_addr_bytes == 0) {
     return first_timing.calculate_timing();
   }

   set_id = get_set_id(second_addr);
   // write to second block
   for (CacheSet& set : sets) {
     if (set.get_set() != set_id) {
       continue;
     }

     unsigned int tag = (second_addr & tag_mask) >> (32 - tag_bits);
     unsigned int b2_word = word >> (first_addr_bytes * 8);
     auto second_timing = set.writeWord(tag, second_addr & block_mask, b2_word, second_addr_bytes);

     first_timing.add(second_timing);
     return first_timing.calculate_timing();
   }

  throw std::runtime_error("Failed to find matching set in cache!!");
  return 8;
}

unsigned int NWayCache::get_set_id(unsigned int address) {
  return (address & set_mask) >> block_bits;
}

unsigned char NWayCache::split_blocks(unsigned int address, unsigned int& outSecondAddr) {
  unsigned int block_address = address & block_mask;

  // not near the end of a block, so no splitting needed
  if (block_mask - block_address >= 3) {
    outSecondAddr = 0;
    return 0;
  }

  unsigned char num_bytes = 3 - (block_mask - block_address);
  outSecondAddr = (address & (~block_mask)) + (1 << block_bits);
  
  return num_bytes;
}

// CacheSet
CacheSet::CacheSet(unsigned int set, unsigned char tag_bits, unsigned char set_bits, std::vector<CacheLine> lines, unsigned char* prog_mem) {
  this->set = set;
  this->lines = lines;
  this->prog_mem = prog_mem;

  this->tag_bits = tag_bits;
  this->set_bits = set_bits;

  this->counter = 0;
}

unsigned int CacheSet::get_set() {
  return set;
}

CacheTime CacheSet::readWord(unsigned int tag, unsigned int bo, unsigned int& outWord) {
  counter += 1;
  CacheTime timing = CacheTime();

  unsigned long lru_num = ULONG_MAX;
  CacheLine* lru_line = &lines.front();
  for (CacheLine& line : lines) {
    // found uninitialized block
    if (!line.isValid()) {
      line.load_block(prog_mem, tag, set, timing);
      outWord = line.readWord(bo, counter);
      return timing;
    }

    // found matching block
    if (line.get_tag() == tag) {
      outWord = line.readWord(bo, counter);
      return timing;
    }

    // track lru cache line
    if (line.get_used() < lru_num) {
      lru_line = &line;
      lru_num = line.get_used();
    }
  }

  // if we get here, the tag did not match any cached blocks
  lru_line->write_block(prog_mem, set, timing);
  lru_line->load_block(prog_mem, tag, set, timing);

  outWord = lru_line->readWord(bo, counter);
  return timing;
}

CacheTime CacheSet::writeWord(unsigned int tag, unsigned int bo, unsigned int word, unsigned char num_bytes) {
  counter += 1;
  CacheTime timing = CacheTime();

  unsigned long lru_num = ULONG_MAX;
  CacheLine* lru_line = &lines.front();
  for (CacheLine& line : lines) {
    // found uninitialized block
    if (!line.isValid()) {
      line.load_block(prog_mem, tag, set, timing);
      line.writeWord(bo, word, counter, num_bytes);
      return timing;
    }

    // found matching block
    if (line.get_tag() == tag) {
      line.writeWord(bo, word, counter, num_bytes);
      return timing;
    }

    // track lru cache line
    if (line.get_used() < lru_num) {
      lru_line = &line;
      lru_num = line.get_used();
    }
  }

  // if we get here, the tag did not match any cached blocks
  lru_line->write_block(prog_mem, set, timing);
  lru_line->load_block(prog_mem, tag, set, timing);

  lru_line->writeWord(bo, word, counter, num_bytes);
  return timing;
}



// CacheLine
CacheLine::CacheLine(unsigned int block_size, unsigned char tag_bits, unsigned char bo_bits) {
  block = std::vector<unsigned char>(block_size, 0);

  if (block_size != (1 << bo_bits)) {
    throw std::invalid_argument("block size does not match bo_bits");
  }

  valid = false;
  changed = false;
  tag = 0;
  used = 0;

  this->tag_bits = tag_bits;  
  this->bo_bits = bo_bits;
}

unsigned int CacheLine::get_tag() {
  return tag;
}

unsigned int CacheLine::get_used() {
  return used;
}

bool CacheLine::isValid() {
  return valid;
}


unsigned int CacheLine::readWord(unsigned int block_offset, unsigned long used) {
  this->used = used;
  
  unsigned int word = 0;
  if (block_offset + 3 < block.size()) {
    word += block[block_offset + 3] << 24;
  }
  if (block_offset + 2 < block.size()) {
    word += block[block_offset + 2] << 16;
  }
  if (block_offset + 1 < block.size()) {
    word += block[block_offset + 1] << 8;
  }
  if (block_offset < block.size()) {
    word += block[block_offset];
  }
  return word;
}

void CacheLine::writeWord(unsigned int block_offset, unsigned int word, unsigned long used, unsigned char num_bytes) {
  changed = true;
  this->used = used;

  if (block_offset + 3 < block.size() && num_bytes >= 4) {
    block[block_offset + 3] = (word >> 24) & 0xFF;
  }
  if (block_offset + 2 < block.size() && num_bytes >= 3) {
    block[block_offset + 2] = (word >> 16) & 0xFF;
  }
  if (block_offset + 1 < block.size() && num_bytes >= 2) {
    block[block_offset + 1] = (word >> 8) & 0xFF;
  }
  if (block_offset < block.size() && num_bytes >= 1) {
    block[block_offset] = word & 0xFF;
  }
  return;
}

void CacheLine::write_block(unsigned char* prog_mem, unsigned int set, CacheTime& timing) {
  // if nothing changed no need to write back 
  if (!changed) {
    return;
  }

  unsigned int block_address = assemble_block_address(tag, set);

  for (auto i = 0; i < block.size(); i++) {
    prog_mem[block_address + i] = block[i];
  }

  timing.words_stored += block.size() / 4;
}

void CacheLine::load_block(unsigned char* prog_mem, unsigned int tag, unsigned int set, CacheTime& timing) {
  valid = true;
  changed = false;

  this->tag = tag;  

  unsigned int block_address = assemble_block_address(tag, set);

  // read from memory into block
  for (auto i = 0; i < block.size(); i++) {
    block[i] = prog_mem[block_address + i];
  }

  timing.words_loaded += block.size() / 4;
}

unsigned int CacheLine::assemble_block_address(unsigned int tag, unsigned int set) {
    return ((tag << (32 - tag_bits)) | (set << bo_bits));
}

