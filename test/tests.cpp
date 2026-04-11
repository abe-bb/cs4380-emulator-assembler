#include "gtest/gtest.h"
#include <cstdint>
#include <cstdlib>
#include <gtest/gtest.h>
#include <vector>

#include "../include/emu4380.h"

// helper function for setting control register operands
void set_operands(unsigned int operand1, unsigned int operand2 = R0, unsigned int operand3 = R0) {
  cntrl_regs[OPERAND_1] = operand1;
  cntrl_regs[OPERAND_2] = operand2;
  cntrl_regs[OPERAND_3] = operand3;
}

// helper function for setting control register operation
void set_operation(unsigned int operation) {
  cntrl_regs[OPERATION] = operation;
}

// helper function for setting control register immediate value
void set_immediate(unsigned int immediate) {
  cntrl_regs[IMMEDIATE] = immediate;
}

TEST(Setup, TestMemoryInit) {
  init_mem(1024);
  init_cache(0);

  for(unsigned int i = 0; i < MEM_SIZE; i++) {
    ASSERT_EQ(prog_mem[i], 0);
  }
}

// out of bounds memory fetch should fail
TEST(Fetch, OutOfBoundsAddressFails) {
  init_mem(1024);

  for (auto i = 0; i < 4; i++) {
    init_cache(i);
    reg_file[PC] = MEM_SIZE + 1;
    ASSERT_FALSE(fetch());
  }
}

TEST(Fetch, Last7BytesFetchFails) {
  init_mem(1024);

  for (auto cache_t = 0; cache_t < 4; cache_t++) {
    init_cache(cache_t);
    for (int i = 1; i < 8; i++) {
      reg_file[PC] = MEM_SIZE - i;
      ASSERT_FALSE(fetch()) << "fetch succeed with address: " << MEM_SIZE - i << " and MEM_SIZE: " << MEM_SIZE;
    }
  }
}

// fetch from valid memory location should succeed
TEST(Fetch, ValidAddressSucceeds) {
  init_mem(1024);

  for (auto i = 0; i < 4; i++) {
    init_cache(i);

    // set the program counter to the 5th byte of memory
    reg_file[PC] = 4;

    ASSERT_TRUE(fetch());
  }
}

// validate that the bytes placed in the contrrol registers are correct (and in correct little endian order)
TEST(Fetch, BytesPlacedInCtrlRegs) {
  init_mem(10000);
  for (auto i = 0; i < 4; i++) {
    init_cache(i);

    // set the program counter to the 5th byte of memory
    reg_file[PC] = 4;


    // set some values o be read into the ctrl registers
    // operation
    prog_mem[4] =  0x01;
    // operand 1
    prog_mem[5] =  0x02;
    // operand 2
    prog_mem[6] =  0x04;
    // operand 3
    prog_mem[7] =  0x08;
    // immediate
    prog_mem[8] =  0xEF;
    prog_mem[9] =  0xBE;
    prog_mem[10] = 0xAD;
    prog_mem[11] = 0xDE;

    // fetch instruction bytes and place in ctrl registers
    EXPECT_TRUE(fetch());

    // check ctrl register values
    EXPECT_EQ(0x01, cntrl_regs[OPERATION]) << "Operator value incorrectly loaded";
    EXPECT_EQ(0x02, cntrl_regs[OPERAND_1]) << "Operand 1 value incorrectly loaded";
    EXPECT_EQ(0x04, cntrl_regs[OPERAND_2]) << "Operand 2 value incorreclty loaded";
    EXPECT_EQ(0x08, cntrl_regs[OPERAND_3]) << "Operand 3 value incorrectly loaded";
    EXPECT_EQ(0xDEADBEEF, cntrl_regs[IMMEDIATE]) << "Immediate value incorrectly loaded";
  }
}

TEST(MemTests, ReadByteTests) {
  auto mem_size = 1024;
  init_mem(mem_size);
  init_cache(0);

  auto mem_cntr_before = mem_cycle_cntr;

  for (auto addr = 0; addr < mem_size; addr++) {
    unsigned char mem = rand();
    prog_mem[addr] = mem;

    auto result = readByte(addr);

    ASSERT_EQ(mem, result);
  }

  ASSERT_EQ(mem_cntr_before + (mem_size * 8), mem_cycle_cntr);
}

TEST(MemTests, ReadWordTests) {
  auto mem_size = 1024;
  init_mem(mem_size);
  init_cache(0);

  auto mem_cntr_before = mem_cycle_cntr;

  for (auto addr = 0; addr < mem_size - 3; addr++) {
    unsigned int mem = rand();
    *(unsigned int*)(prog_mem + addr) = mem;

    auto result = readWord(addr);

    ASSERT_EQ(mem, result);
  }

  ASSERT_EQ(mem_cntr_before + ((mem_size - 3) * 8), mem_cycle_cntr);
}

TEST(MemTests, WriteByteTests) {
  auto mem_size = 1024;
  init_mem(mem_size);
  init_cache(0);

  auto mem_cntr_before = mem_cycle_cntr;

  for (auto addr = 0; addr < mem_size; addr++) {
    unsigned char mem = rand();
    writeByte(addr, mem);

    auto result = prog_mem[addr];
    ASSERT_EQ(mem, result);
  }

  ASSERT_EQ(mem_cntr_before + (mem_size * 8), mem_cycle_cntr);
}

TEST(MemTests, WriteWordTests) {
  auto mem_size = 1024;
  init_mem(mem_size);
  init_cache(0);

  auto mem_cntr_before = mem_cycle_cntr;

  for (auto addr = 0; addr < mem_size - 3; addr++) {
    unsigned int mem = rand();
    writeWord(addr, mem);

    auto result = *(unsigned int*)(prog_mem + addr);
    ASSERT_EQ(mem, result);
  }

  ASSERT_EQ(mem_cntr_before + ((mem_size - 3) * 8), mem_cycle_cntr);
}

TEST(JumpTests, JumpInstructionEnumValues){
  ASSERT_EQ(1, JMP);
  ASSERT_EQ(2, JMR);
  ASSERT_EQ(3, BNZ);
  ASSERT_EQ(4, BGT);
  ASSERT_EQ(5, BLT);
  ASSERT_EQ(6, BRZ);
}

TEST(JumpTests, JumpBeyondMemoryFails) {
  init_mem(1024);

  for (auto i = 0; i < 4; i++) {
    init_cache(i);
    set_immediate(1024);
    set_operands(R8);
    reg_file[R8] = 1024;

    unsigned int jump_ops[] = { JMP, JMR, BNZ, BGT, BLT, BRZ };

    for (auto op : jump_ops) {
      set_operation(op);

      EXPECT_FALSE(execute()) << "Operation: " << op << " did not fail when jumping beyond memory!";
    }
  }
}

TEST(JumpTests, JumpToLast7BytesFails) {
  init_mem(1024);

  for (auto cache_t = 0; cache_t < 4; cache_t++) {
    init_cache(cache_t);
    set_operands(R8);

    unsigned int jump_ops[] = { JMP, JMR, BNZ, BGT, BLT, BRZ };

    for (auto op : jump_ops) {
      set_operation(op);

      for (int i = 1; i < 8; i++) {
        set_immediate(1024 - i);
        reg_file[R8] = 1024 - i;

        ASSERT_FALSE(execute());
      }
    }
  }
}

TEST(JumpTests, JMR) {
  init_mem(1024);
  for (auto i = 0; i < 4; i++) {
    init_cache(i);

    set_operation(JMR);
    set_operands(R9);
    reg_file[R9] = 10;
  
    ASSERT_TRUE(execute());

    ASSERT_EQ(reg_file[PC], 10) << "JMR failed to set PC";
  }
}

TEST(JumpTests, BNZ) {
  init_mem(1024);
  for (auto i = 0; i < 4; i++) {
    init_cache(i);

    set_operation(BNZ);
    set_operands(R9);
    set_immediate(100);
    reg_file[PC] = 0;
    reg_file[R9] = 0;
  
    ASSERT_TRUE(execute());

    ASSERT_EQ(reg_file[PC], 0) << "BNZ set PC when register == 0";

    reg_file[R9] = 1;

    ASSERT_TRUE(execute());

    ASSERT_EQ(reg_file[PC], 100) << "BNZ failed to set PC when register == 1";
  }
}

TEST(JumpTests, BGT) {
  init_mem(1024);
  for (auto i = 0; i < 4; i++) {
    init_cache(i);

    set_operation(BGT);
    set_operands(R9);
    set_immediate(111);
    reg_file[PC] = 0;
    reg_file[R9] = (unsigned int) -100;
  
    ASSERT_TRUE(execute());

    ASSERT_EQ(reg_file[PC], 0) << "BGT set PC when register == -100";


    reg_file[R9] = 1;

    ASSERT_TRUE(execute());

    ASSERT_EQ(reg_file[PC], 111) << "BGT failed to set PC when register == 1";
  }
}

TEST(JumpTests, BLT) {
  init_mem(1024);
  for (auto i = 0; i < 4; i++) {
    init_cache(i);

    set_operation(BLT);
    set_operands(R9);
    set_immediate(112);
    reg_file[PC] = 0;
    reg_file[R9] = 0;
  
    ASSERT_TRUE(execute());

    ASSERT_EQ(reg_file[PC], 0) << "BLT set PC when register == 0";


    reg_file[R9] = (unsigned int) -1;

    ASSERT_TRUE(execute());

    ASSERT_EQ(reg_file[PC], 112) << "BLT failed to set PC when register == -1";
  }
}

TEST(JumpTests, BRZ) {
  init_mem(1024);
  for (auto i = 0; i < 4; i++) {
    init_cache(i);

    set_operation(BRZ);
    set_operands(R9);
    set_immediate(113);
    reg_file[PC] = 0;
    reg_file[R9] = 1;
  
    ASSERT_TRUE(execute());

    ASSERT_EQ(reg_file[PC], 0) << "BRZ set PC when register == 1";


    reg_file[R9] = 0;

    ASSERT_TRUE(execute());

    ASSERT_EQ(reg_file[PC], 113) << "BRZ failed to set PC when register == 0";
  }
}


// tests that fetch properly increments PC
TEST(Fetch, IncrementsPC) {
  init_mem(1024);
  for (auto i = 0; i < 4; i++) {
    init_cache(i);
    reg_file[PC] = 0;
    ASSERT_TRUE(fetch());
    EXPECT_EQ(reg_file[PC], 8);
  }
}

TEST(Decode, SingleDontCareValuesShouldntFail) {
  for (auto operation : operations_2operand_1dc) {
    for (unsigned int invalid_operand = 22; invalid_operand < 256; invalid_operand++) {
      set_operation(operation);
      set_operands(R0, R1, invalid_operand);

      ASSERT_TRUE(decode()) << "Operation: " << operation << " with 1 Don't Care register value: " << invalid_operand << " caused decode failure";
    }
  }
}

TEST(Decode, DoubleDontCareValuesShouldntFail) {
  for (auto operation : operations_1operand_2dc) {
    for (unsigned int invalid_operand = 22; invalid_operand < 256; invalid_operand++) {
      set_operation(operation);
      set_operands(R0, invalid_operand, invalid_operand);

      ASSERT_TRUE(decode()) << "Operation with 2 Don't Care register value caused decode failure";
    }
  }
}

TEST(Decode, TripleDontCareValuesShouldntFail) {
  set_immediate(0);

  for (auto operation : operations_0operand_3dc) {
    for (unsigned int invalid_operand = 22; invalid_operand < 256; invalid_operand++) {
      set_operation(operation);
      set_operands(invalid_operand, invalid_operand, invalid_operand);

      ASSERT_TRUE(decode()) << "Operation: " << operation << " with 3 Don't Care register value caused decode failure";
    }
  }
}

TEST(Decode, InvalidOperandsFail) {
  // concatenate all vectors with operands
  std::vector<unsigned int> operations_with_operands(operations_1operand_2dc.begin(), operations_1operand_2dc.end());
  operations_with_operands.insert(operations_with_operands.end(), operations_2operand_1dc.begin(), operations_2operand_1dc.end());
  operations_with_operands.insert(operations_with_operands.end(), operations_3operand_0dc.begin(), operations_3operand_0dc.end());

  for (unsigned int operation: operations_with_operands) {
    // loop over invalid operand values
    for (unsigned int invalid_operand = 22; invalid_operand < 256; invalid_operand++) {
      set_operation(operation);
      set_operands(invalid_operand, invalid_operand, invalid_operand);

      ASSERT_FALSE(decode()) << "decode on operation: " << operation << " with all operands set to: " <<
      invalid_operand << " succeeded despite invalid operands";
    }
  }
}

// iterate through valid operations and assign every valid operand to each, ensuring that decode succeeds
TEST(Decode, ValidOperandsSucceed) {
  uint8_t max_valid_operand = 15;

  // 3 operands 0 don't care
  for (unsigned int operation: operations_3operand_0dc) {
    for (unsigned int operand1 = 0; operand1 <= max_valid_operand; operand1++) {
      unsigned int operand2 = (operand1 + 1) % max_valid_operand;
      unsigned int operand3 = (operand1 + 2) % max_valid_operand;
      set_operands(operand1, operand2, operand3);
      set_operation(operation);

      ASSERT_TRUE(decode()) << "Operation: " << operation << " with Operand_1: " << operand1 <<
      " and Operand_2: " << operand2 << " failed to decode";
    }
  }

  // 2 operand 1 don't care
  for (unsigned int operation: operations_2operand_1dc) {
    for (unsigned int operand1 = 0; operand1 <= max_valid_operand; operand1++) {
      unsigned int operand2 = (operand1 + 1) % max_valid_operand;
      set_operands(operand1, operand2);
      set_operation(operation);

      ASSERT_TRUE(decode()) << "Operation: " << operation << " with Operand_1: " << operand1 <<
      " and Operand_2: " << operand2 << " failed to decode";
    }
  }

  // 1 operand 2 don't care
  for (unsigned int operation: operations_1operand_2dc) {
    for (int operand = 0; operand <= max_valid_operand; operand++) {
      set_operands(operand);
      set_operation(operation);

      ASSERT_TRUE(decode()) << "Operation: " << operation << " with Operand_1: " << operand << " failed to decode";
    }
  }
}

// iterate through all valid operations and ensure that they all decode successfully
TEST(Decode, ValidOperationsSucceed) {
  init_mem(1024);
  init_cache(0);
  set_operands(R0, R1, R2);
  set_immediate(4);

  // list valid operator values
  std::vector<unsigned int> valid_operations = std::vector<unsigned int>();
  valid_operations.insert(valid_operations.end(), operations_0operand_3dc.begin(), operations_0operand_3dc.end());
  valid_operations.insert(valid_operations.end(), operations_1operand_2dc.begin(), operations_1operand_2dc.end());
  valid_operations.insert(valid_operations.end(), operations_2operand_1dc.begin(), operations_2operand_1dc.end());
  valid_operations.insert(valid_operations.end(), operations_3operand_0dc.begin(), operations_3operand_0dc.end());


  // loop over all valid operators and decode them
  for (unsigned int i : valid_operations) {
    // set the OPERATION control register
    cntrl_regs[OPERATION] = i;

    ASSERT_TRUE(decode()) << "operation: " << i << " failed to decode when valid";
  }
}

// test a variety of invalid operations and ensure that they all fail to decode
TEST(Decode, InvalidOperationsFail) {
  set_operands(R0, R1, R2);
  set_immediate(4);

  // list invalid operator values to be tested
  std::vector<unsigned int> invalid_operations = {0, 27, 28, 32, 33, 55, 100, 200, 255};

  // loop over all valid operators and decode them
  for (unsigned int i : invalid_operations) {
    // set the OPERATION control register
    cntrl_regs[OPERATION] = i;

    EXPECT_FALSE(decode()) << "decode for operation: " << i << "returned success (invalid operation)";
  }
}

TEST(Decode, ValidTRPSucceeds) {
  unsigned int valid_trp[] = {0, 1, 2, 3, 4, 98};

  for (unsigned int trp : valid_trp) {
    set_operation(TRP);
    set_immediate(trp);

    ASSERT_TRUE(decode());
  }
}

TEST(Decode, InvalidTRPFails) {

  for (unsigned int i = 5; i < 98; i++) {
    set_operation(TRP);
    set_immediate(i);

    ASSERT_FALSE(decode());
  }

  for (unsigned int i = 99; i < 255; i++) {
    set_operation(TRP);
    set_immediate(i);

    ASSERT_FALSE(decode());
  }
}

TEST(ExecuteCompare, CmpFunctionality) {
  set_operation(CMP);
  set_operands(R4, R5, R6);

  reg_file[R4] = 100;
  reg_file[R5] = -50;
  reg_file[R6] = -50;

  EXPECT_TRUE(execute());
  EXPECT_EQ(0, reg_file[R4]) << "operands are equal, should be 0";

  reg_file[R5] = 50;
  EXPECT_TRUE(execute());
  EXPECT_EQ(1, reg_file[R4]) << "operands are equal, should be 0";

  reg_file[R5] = -500;
  EXPECT_TRUE(execute());
  EXPECT_EQ(-1, reg_file[R4]) << "operands are equal, should be 0";
}

TEST(ExecuteCompare, CmpiFunctionality) {
  set_operation(CMPI);
  set_operands(R4, R5);
  set_immediate(-50);

  reg_file[R4] = 100;
  reg_file[R5] = -50;
  

  EXPECT_TRUE(execute());
  EXPECT_EQ(0, reg_file[R4]) << "operands are equal, should be 0";

  reg_file[R5] = 50;
  EXPECT_TRUE(execute());
  EXPECT_EQ(1, reg_file[R4]) << "operands are equal, should be 0";

  reg_file[R5] = -500;
  EXPECT_TRUE(execute());
  EXPECT_EQ(-1, reg_file[R4]) << "operands are equal, should be 0";
}

TEST(ExecuteFlow, JumpSetsPC) {
  init_mem(1024);
  for (auto i = 0; i < 4; i++) {
    init_cache(i);
    set_operation(JMP);
    set_immediate(72);

    EXPECT_TRUE(execute());
    EXPECT_EQ(72, reg_file[PC]);
  }
}

TEST(ExecuteFlow, JumpBeyondMemoryFails) {
  init_mem(1024);
  for (auto cache_t = 0; cache_t < 4; cache_t++) {
    init_cache(cache_t);
    set_operation(JMP);
    set_immediate(1024);

    EXPECT_FALSE(execute());
  }
}

TEST(ExecuteFlow, JumpToLast7BytesFails) {
  init_mem(1024);
  for (auto cache_t = 0; cache_t < 4; cache_t++) {
    init_cache(cache_t);

    set_operation(JMP);

    for (int i = 1; i < 8; i++) {
      set_immediate(1024 - i);
      ASSERT_FALSE(execute());
    }
  }
}

TEST(ExecuteMove, IstrStoresInteger) {
  init_mem(1024);
  for (auto cache_t = 0; cache_t < 4; cache_t++) {
    init_cache(cache_t);
    set_operation(ISTR);
    set_operands(R7, R8);

    // set address
    reg_file[R8] = 503;

    // set value to be stored
    reg_file[R7] = 0x87654321;

    EXPECT_TRUE(execute());

    auto msg = "ISTR failed to properly store int to memory location. mem: 0x";

    // test if the value saved correctly in little endian order
    EXPECT_EQ(0x21, (int)prog_mem[503]) << msg << std::hex << prog_mem[503];
    EXPECT_EQ(0x43, (int)prog_mem[504]) << msg << std::hex << prog_mem[504];
    EXPECT_EQ(0x65, (int)prog_mem[505]) << msg << std::hex << prog_mem[505];
    EXPECT_EQ(0x87, (int)prog_mem[506]) << msg << std::hex << prog_mem[506];
  }
}

TEST(ExecuteMove, IldrLoadsInteger) {
  init_mem(1024);
  for (auto cache_t = 0; cache_t < 4; cache_t++) {
    init_cache(cache_t);
    set_operation(ILDR);
    set_operands(R7, R8);

    // set address
    reg_file[R8] = 503;

    // clear R7
    reg_file[R7] = 0;

    // set values to load in memory
    prog_mem[503] = 0x21;
    prog_mem[504] = 0x43;
    prog_mem[505] = 0x65;
    prog_mem[506] = 0x87;


    EXPECT_TRUE(execute());

    auto msg = "ILDR failed to properly load int from memory location\n";

    // test if the value saved correctly in little endian order
    ASSERT_EQ(0x87654321, reg_file[R7]) << msg;
  }
}

TEST(ExecuteMove, IstbStoresByte) {
  init_mem(1024);
  for (auto cache_t = 0; cache_t < 4; cache_t++) {
    init_cache(cache_t);
    set_operation(ISTB);
    set_operands(R7, R8);

    // set address
    reg_file[R8] = 509;

    // set value to be stored
    reg_file[R7] = 0xAC;

    EXPECT_TRUE(execute());

    auto msg = "ISTB failed to properly store byte to memory location\n";

    // test if the value saved correctly in little endian order
    EXPECT_EQ(0xAC, prog_mem[509]) << msg;
  }
}

TEST(ExecuteMove, IldbLoadsByte) {
  init_mem(1024);
  for (auto cache_t = 0; cache_t < 4; cache_t++) {
    init_cache(cache_t);
    set_operation(ILDB);
    set_operands(R7, R8);

    // set address
    reg_file[R8] = 510;

    // clear R7
    reg_file[R7] = 0;

    // set values to load in memory
    prog_mem[510] = 0xAD;


    EXPECT_TRUE(execute());

    auto msg = "ILDB failed to properly load byte from memory location\n";

    // test if the value saved correctly in little endian order
    ASSERT_EQ(0xAD, reg_file[R7]) << msg;
  }
}

TEST(ExecuteMove, MovCopiesContents) {
  set_operation(MOV);
  set_operands(R1, R2);
  reg_file[R2] = 0x0FF1CE;

  EXPECT_TRUE(execute());
  ASSERT_EQ(0x0FF1CE, reg_file[R1]);
}

TEST(ExecuteMove, MoviPutsImmediateInRegister) {
  set_operation(MOVI);
  set_operands(R5);
  set_immediate(0x45FA78ED);

  reg_file[R5] = 0x00;

  EXPECT_TRUE(execute());

  ASSERT_EQ(0x45FA78ED, reg_file[R5]) << "MOVI did not store immediate value in the specified register";
}

TEST(ExecuteMove, LdaPlacesAddressInRegister) {
  init_mem(1024);
  for (auto cache_t = 0; cache_t < 4; cache_t++) {
    init_cache(cache_t);
    set_operation(LDA);
    set_operands(R6);
    set_immediate(157);

    prog_mem[157] = 255;

    EXPECT_TRUE(execute());

    ASSERT_EQ(157, reg_file[R6]) << "LDA failed to load memory contents to register";
  }
}

// loads the value from memory into a register
TEST(ExecuteMove, TestLDA32BitLoad) {
  init_mem(1024);
  for (auto cache_t = 0; cache_t < 4; cache_t++) {
    init_cache(cache_t);

    set_operation(LDA);
    set_operands(R0);
    set_immediate(0xDEADBEEF);

    ASSERT_TRUE(execute());
    EXPECT_EQ(0xDEADBEEF, reg_file[R0]);
  }
}

TEST(ExecuteMove, StrStoresIntToMemory) {
  init_mem(1024);
  for (auto cache_t = 0; cache_t < 4; cache_t++) {
    init_cache(cache_t);
    set_operation(STR);
    set_operands(R7);
    set_immediate(503);

    // set value to be stored
    reg_file[R7] = 0x87654321;

    EXPECT_TRUE(execute());

    auto msg = "STR failed to properly store int to memory location. mem: 0x";

    // test if the value saved correctly in little endian order
    EXPECT_EQ(0x21, (int)prog_mem[503]) << msg << std::hex << (int)prog_mem[503];
    EXPECT_EQ(0x43, (int)prog_mem[504]) << msg << std::hex << (int)prog_mem[504];
    EXPECT_EQ(0x65, (int)prog_mem[505]) << msg << std::hex << (int)prog_mem[505];
    EXPECT_EQ(0x87, (int)prog_mem[506]) << msg << std::hex << (int)prog_mem[506];
  }
}

TEST(ExecuteMove, StrFailsBeyondMemory) {
  init_mem(1024);
  for (auto cache_t = 0; cache_t < 4; cache_t++) {
    init_cache(cache_t);
    set_operation(STR);
    set_operands(R6);
    set_immediate(1024);

    ASSERT_FALSE(execute());
  }
}

TEST(ExecuteMove, StrFailsLast3Addresses) {
  init_mem(1024);
  for (auto cache_t = 0; cache_t < 4; cache_t++) {
    init_cache(cache_t);
    set_operation(STR);
    set_operands(R6);

  
    for (int i = 3; i<4; i++) {
      set_immediate(1024 - i);
      ASSERT_FALSE(execute());
    }
  }
}


TEST(ExecuteMove, TestLdr) {
  init_mem(1024);
  for (auto cache_t = 0; cache_t < 4; cache_t++) {
    init_cache(cache_t);

    prog_mem[60] = 0x21;
    prog_mem[61] = 0x43;
    prog_mem[62] = 0x65;
    prog_mem[63] = 0x87;

    set_operation(LDR);
    set_operands(R0);
    set_immediate(60);

    ASSERT_TRUE(execute());
    // check 0x87654321 interpreted as two's compliment
    EXPECT_EQ(-2023406815, (int) reg_file[R0]);
  }
}

TEST(ExecuteMove, LdrFailsBeyondMemory) {
  init_mem(1024);
  for (auto cache_t = 0; cache_t < 4; cache_t++) {
    init_cache(cache_t);
    set_operation(LDR);
    set_operands(R6);
    set_immediate(1024);

    ASSERT_FALSE(execute());
  }
}

TEST(ExecuteMove, LdrFailsLast3Addresses) {
  init_mem(1024);
  for (auto cache_t = 0; cache_t < 4; cache_t++) {
    init_cache(cache_t);
    set_operation(LDR);
    set_operands(R6);

  
    for (int i = 3; i<4; i++) {
      set_immediate(1024 - i);
      ASSERT_FALSE(execute());
    }
  }
}

TEST(ExecuteMove, TestStb) {
  init_mem(1024);
  for (auto cache_t = 0; cache_t < 4; cache_t++) {
    init_cache(cache_t);

    reg_file[R0] = 0xCD;

    set_operation(STB);
    set_operands(R0);
    set_immediate(60);

    ASSERT_TRUE(execute());
    EXPECT_EQ(0xCD, prog_mem[60]);
  }
}

TEST(ExecuteMove, TestStbStores1Byte) {
  init_mem(1024);
  // write through to memory
  init_cache(0);

  reg_file[R0] = 0x87654321;

  set_operation(STB);
  set_operands(R0);
  set_immediate(60);

  prog_mem[61] = 0;
  prog_mem[62] = 0;
  prog_mem[63] = 0;
  prog_mem[57] = 0;
  prog_mem[58] = 0;
  prog_mem[59] = 0;

  ASSERT_TRUE(execute());
  EXPECT_EQ(0x21, prog_mem[60]);
  EXPECT_EQ(0, prog_mem[61]);
  EXPECT_EQ(0, prog_mem[62]);
  EXPECT_EQ(0, prog_mem[63]);
  EXPECT_EQ(0, prog_mem[57]);
  EXPECT_EQ(0, prog_mem[58]);
  EXPECT_EQ(0, prog_mem[59]);
}

TEST(ExecuteMove, StbFailsBeyondMemory) {
  init_mem(1024);
  for (auto cache_t = 0; cache_t < 4; cache_t++) {
    init_cache(cache_t);
    set_operation(STB);
    set_operands(R6);
    set_immediate(1024);

    ASSERT_FALSE(execute());
  }
}

TEST(ExecuteMove, TestLdb) {
  init_mem(1024);
  for (auto cache_t = 0; cache_t < 4; cache_t++) {
    init_cache(cache_t);

    prog_mem[60] = 0xCD;

    set_operation(LDB);
    set_operands(R0);
    set_immediate(60);

    ASSERT_TRUE(execute());
    EXPECT_EQ(0xCD, reg_file[R0]);
  }
}

TEST(ExecuteMove, LdbFailsBeyondMemory) {
  init_mem(1024);
  for (auto cache_t = 0; cache_t < 4; cache_t++) {
    init_cache(cache_t);
    set_operation(LDB);
    set_operands(R6);
    set_immediate(1024);

    ASSERT_FALSE(execute());
  }
}

TEST(ExecuteMath, TestADD) {
  set_operation(ADD);
  set_operands(R2, R1, R0);
  set_immediate(0);

  reg_file[R0] = 51;
  reg_file[R1] = 34;

  ASSERT_TRUE(execute());

  ASSERT_EQ(85, reg_file[R2]);
}

TEST(ExecuteMath, TestADDI) {
  set_operation(ADDI);
  set_operands(R2, R1);
  set_immediate(23);

  reg_file[R1] = 44;

  ASSERT_TRUE(execute());

  ASSERT_EQ(67, reg_file[R2]);
}

TEST(ExecuteMath, TestSUB) {
  set_operation(SUB);
  set_operands(R2, R1, R0);
  set_immediate(0);

  reg_file[R0] = 34;
  reg_file[R1] = 51;

  ASSERT_TRUE(execute());

  ASSERT_EQ(17, reg_file[R2]);
}

TEST(ExecuteMath, TestSUBI) {
  set_operation(SUBI);
  set_operands(R2, R1);
  set_immediate(23);

  reg_file[R1] = 44;

  ASSERT_TRUE(execute());

  ASSERT_EQ(21, reg_file[R2]);
}

TEST(ExecuteMath, TestMUL) {
  set_operation(MUL);
  set_operands(R2, R1, R0);
  set_immediate(0);

  reg_file[R0] = 51;
  reg_file[R1] = 34;

  ASSERT_TRUE(execute());

  ASSERT_EQ(1734, reg_file[R2]);
}

TEST(ExecuteMath, TestMULI) {
  set_operation(MULI);
  set_operands(R2, R1);
  set_immediate(23);

  reg_file[R1] = 44;

  ASSERT_TRUE(execute());

  ASSERT_EQ(1012, reg_file[R2]);
}

TEST(ExecuteMath, TestDiv) {
  set_operation(DIV);
  set_operands(R2, R1, R0);
  set_immediate(0);

  reg_file[R1] = 109568;
  reg_file[R0] = 428;

  ASSERT_TRUE(execute());

  ASSERT_EQ(256, reg_file[R2]);
}

TEST(ExecuteMath, TestDivByZeroFails) {
  set_operation(DIV);
  set_operands(R2, R1, R0);
  set_immediate(0);

  reg_file[R1] = 109568;
  reg_file[R0] = 0;

  EXPECT_FALSE(execute());

  set_operation(SDIV);
  EXPECT_FALSE(execute());

  
  set_operation(DIVI);
  EXPECT_FALSE(execute());
}

TEST(ExecuteMath, TestSdiv) {
  set_operation(SDIV);
  set_operands(R2, R1, R0);
  set_immediate(0);

  reg_file[R1] = -109568;
  reg_file[R0] = 428;

  ASSERT_TRUE(execute());

  ASSERT_EQ(-256, (int)reg_file[R2]);
}

TEST(ExecuteMath, TestSdivByZeroFails) {
  set_operation(SDIV);
  set_operands(R2, R1, R0);
  set_immediate(0);

  reg_file[R1] = -109568;
  reg_file[R0] = 0;

  ASSERT_FALSE(execute());
}

TEST(ExecuteMath, TestDivi) {
  set_operation(DIVI);
  set_operands(R2, R1);
  set_immediate(428);

  reg_file[R1] = 109568;

  ASSERT_TRUE(execute());

  ASSERT_EQ(256, reg_file[R2]);
}

TEST(ExecuteMath, TestSignedDivi) {
  set_operation(DIVI);
  set_operands(R2, R1);

  set_immediate(-428);
  reg_file[R1] = 109568;
  EXPECT_TRUE(execute());
  EXPECT_EQ(-256, reg_file[R2]);


  reg_file[R1] = -109568;
  EXPECT_TRUE(execute());
  EXPECT_EQ(256, reg_file[R2]);

  set_immediate(428);
  EXPECT_TRUE(execute());
  EXPECT_EQ(-256, reg_file[R2]);
}

TEST(ExecuteMath, TestDiviByZeroFails) {
  set_operation(DIVI);
  set_operands(R2, R1);
  set_immediate(0);

  reg_file[R1] = 109568;

  ASSERT_FALSE(execute());
}

TEST(ExecuteTRP, TRP4ReadsChar) {
    // Create pipe to mock stdin
    int fildes[2];
    int status = pipe(fildes);
    ASSERT_NE(status, -1);

    // Swap `stdin` fd with the "read" end of the pipe
    status = dup2(fildes[0], STDIN_FILENO);
    ASSERT_NE(status, -1);

    // Create payload
    const char buf[] = "c\n";
    const int bsize  = strlen(buf);

    // Send payload through pipe
    ssize_t nbytes = write(fildes[1], buf, bsize);
    close(fildes[1]);

    set_operation(TRP);
    set_immediate(4);

    // execute
    ASSERT_TRUE(execute());

    // check that R3 has teh right value
    ASSERT_EQ('c', reg_file[R3]);
}

TEST(ExecuteTRP, TRP4ReadsSuccessiveChars) {
    // Create pipe to mock stdin
    int fildes[2];
    int status = pipe(fildes);
    ASSERT_NE(status, -1);

    // connect read end of pipe and stdin
    status = dup2(fildes[0], STDIN_FILENO);
    ASSERT_NE(status, -1);

    // Create payload
    const char buf[] = "asdfabcdefghijklmnopqrstuvwxyzABCDXYZ";
    const int bsize  = strlen(buf);

    // Send payload through pipe
    ssize_t nbytes = write(fildes[1], buf, bsize);
    close(fildes[1]);

    set_operation(TRP);
    set_immediate(4);

    for (int i = 0; i < strlen(buf); i++) {
      // execute
      ASSERT_TRUE(execute());

      // check that R3 has teh right value
      ASSERT_EQ(buf[i], reg_file[R3]);
    }
}

TEST(ExecuteTRP, ExecuteTrpNeverExits) {
  set_operation(TRP);

  set_immediate(0);
  execute();
}

// ISTR
TEST(PeerTest, TestIstrInstruction) {
    init_mem(1024);
    for (auto cache_t = 0; cache_t < 4; cache_t++) {
      init_cache(cache_t);
      reg_file[R1] = 0xAABBCCDD;
      reg_file[R2] = 100;

      cntrl_regs[OPERATION] = ISTR;
      cntrl_regs[OPERAND_1] = R1;
      cntrl_regs[OPERAND_2] = R2;

      EXPECT_TRUE(execute());

      EXPECT_EQ(prog_mem[100], 0xDD);
      EXPECT_EQ(prog_mem[101], 0xCC);
      EXPECT_EQ(prog_mem[102], 0xBB);
      EXPECT_EQ(prog_mem[103], 0xAA);
    }
}

// ILDR
TEST(PeerTest, TestIldrInstruction) {
    init_mem(1024);
    for (auto cache_t = 0; cache_t < 4; cache_t++) {
      init_cache(cache_t);
      prog_mem[200] = 0x11;
      prog_mem[201] = 0x22;
      prog_mem[202] = 0x33;
      prog_mem[203] = 0x44;
      reg_file[R2] = 200;

      cntrl_regs[OPERATION] = ILDR;
      cntrl_regs[OPERAND_1] = R1;
      cntrl_regs[OPERAND_2] = R2;

      EXPECT_TRUE(execute());

      EXPECT_EQ(reg_file[R1], 0x44332211u);
    }
}

// ISTB
TEST(PeerTest, TestIstbInstruction) {
    init_mem(1024);
    for (auto cache_t = 0; cache_t < 4; cache_t++) {
      init_cache(cache_t);
      reg_file[R1] = 0xAABBCCDD;
      reg_file[R2] = 300;

      cntrl_regs[OPERATION] = ISTB;
      cntrl_regs[OPERAND_1] = R1;
      cntrl_regs[OPERAND_2] = R2;

      EXPECT_TRUE(execute());

      EXPECT_EQ(prog_mem[300], 0xDD);
    }
}

// ILDB
TEST(PeerTest, TestIldbInstruction) {
    init_mem(1024);
    for (auto cache_t = 0; cache_t < 4; cache_t++) {
      init_cache(cache_t);
      prog_mem[400] = 0xAB;
      reg_file[R2] = 400;

      cntrl_regs[OPERATION] = ILDB;
      cntrl_regs[OPERAND_1] = R1;
      cntrl_regs[OPERAND_2] = R2;

      EXPECT_TRUE(execute());

      EXPECT_EQ(reg_file[R1], 0xABu);
    }
}

// JMR
TEST(PeerTest, TestJmrInstruction) {
    init_mem(1024);
    for (auto cache_t = 0; cache_t < 4; cache_t++) {
      init_cache(cache_t);
      reg_file[R1] = 256;

      cntrl_regs[OPERATION] = JMR;
      cntrl_regs[OPERAND_1] = R1;

      EXPECT_TRUE(execute());

      EXPECT_EQ(reg_file[PC], 256u);
    }
}

// BNZ
TEST(PeerTest, TestBnzInstruction) {
    init_mem(1024);
    for (auto cache_t = 0; cache_t < 4; cache_t++) {
      init_cache(cache_t);
      reg_file[R1] = 5;

      cntrl_regs[OPERATION] = BNZ;
      cntrl_regs[OPERAND_1] = R1;
      cntrl_regs[IMMEDIATE] = 300;

      EXPECT_TRUE(execute());

      EXPECT_EQ(reg_file[PC], 300u);
    }
}

// BGT
TEST(PeerTest, TestBgtInstruction) {
    init_mem(1024);
    for (auto cache_t = 0; cache_t < 4; cache_t++) {
      init_cache(cache_t);
      reg_file[R1] = static_cast<unsigned int>(10);

      cntrl_regs[OPERATION] = BGT;
      cntrl_regs[OPERAND_1] = R1;
      cntrl_regs[IMMEDIATE] = 400;

      EXPECT_TRUE(execute());

      EXPECT_EQ(reg_file[PC], 400u);
    }
}

// BLT
TEST(PeerTest, TestBltInstruction) {
    init_mem(1024);
    for (auto cache_t = 0; cache_t < 4; cache_t++) {
      init_cache(cache_t);
      reg_file[R1] = static_cast<unsigned int>(-3);

      cntrl_regs[OPERATION] = BLT;
      cntrl_regs[OPERAND_1] = R1;
      cntrl_regs[IMMEDIATE] = 500;

      EXPECT_TRUE(execute());

      EXPECT_EQ(reg_file[PC], 500u);
    }
}

// BRZ
TEST(PeerTest, TestBrzInstruction) {
    init_mem(1024);
    for (auto cache_t = 0; cache_t < 4; cache_t++) {
      init_cache(cache_t);
      reg_file[R1] = 0;

      cntrl_regs[OPERATION] = BRZ;
      cntrl_regs[OPERAND_1] = R1;
      cntrl_regs[IMMEDIATE] = 600;

      EXPECT_TRUE(execute());

      EXPECT_EQ(reg_file[PC], 600u);
    }
}

// CMP
TEST(PeerTest, TestCmpInstruction) {
    reg_file[R1] = static_cast<unsigned int>(10);
    reg_file[R2] = static_cast<unsigned int>(5);

    cntrl_regs[OPERATION] = CMP;
    cntrl_regs[OPERAND_1] = R3;
    cntrl_regs[OPERAND_2] = R1;
    cntrl_regs[OPERAND_3] = R2;

    EXPECT_TRUE(execute());

    EXPECT_EQ(static_cast<int>(reg_file[R3]), 1);
}

// CMPI
TEST(PeerTest, TestCmpiInstruction) {
    reg_file[R1] = static_cast<unsigned int>(5);

    cntrl_regs[OPERATION] = CMPI;
    cntrl_regs[OPERAND_1] = R3;
    cntrl_regs[OPERAND_2] = R1;
    cntrl_regs[IMMEDIATE] = 10;

    EXPECT_TRUE(execute());

    EXPECT_EQ(static_cast<int>(reg_file[R3]), -1);
}

//MOVI test
TEST(PeerTest, MOVIInstruction) {
    cntrl_regs[OPERATION]=MOVI;
    cntrl_regs[OPERAND_1]=R1;
    cntrl_regs[IMMEDIATE]=42;
    ASSERT_TRUE(execute());
    ASSERT_EQ(reg_file[R1], 42);
}

//ADD test
TEST(PeerTest, ADDInstruction) {
    reg_file[R1]=10;
    reg_file[R2]=5;
    cntrl_regs[OPERATION]=ADD;
    cntrl_regs[OPERAND_1]=R3;
    cntrl_regs[OPERAND_2]=R1;
    cntrl_regs[OPERAND_3]=R2;
    ASSERT_TRUE(execute());
    ASSERT_EQ(reg_file[R3], 15);
}

//SUB instruction
TEST(PeerTest, SUBInstruction) {
    reg_file[R1]=10;
    reg_file[R2]=3;
    cntrl_regs[OPERATION]=SUB;
    cntrl_regs[OPERAND_1]=R3;
    cntrl_regs[OPERAND_2]=R1;
    cntrl_regs[OPERAND_3]=R2;
    ASSERT_TRUE(execute());
    ASSERT_EQ(reg_file[R3], 7);
}
//TRP halt test
TEST(PeerTest, TRP0Halts) {
    cntrl_regs[OPERATION]=TRP;
    cntrl_regs[IMMEDIATE]=0;
    ASSERT_TRUE(execute());
}

TEST(LogicalTest, AND) {
  set_operands(R0, R1, R2);
  set_operation(AND);

  // TRUE && TRUE
  reg_file[R0] = 100;
  reg_file[R1] = 1;
  reg_file[R2] = 1;
  ASSERT_TRUE(execute());
  ASSERT_EQ(1, prog_mem[R0]);

  // TRUE && TRUE
  reg_file[R0] = 100;
  reg_file[R1] = 5234234;
  reg_file[R2] = 2083423;
  ASSERT_TRUE(execute());
  ASSERT_EQ(1, prog_mem[R0]);

  // FALSE && TRUE
  reg_file[R0] = 100;
  reg_file[R1] = 0;
  reg_file[R2] = 2083423;
  ASSERT_TRUE(execute());
  ASSERT_EQ(0, prog_mem[R0]);

  // FALSE && FALSE
  reg_file[R0] = 100;
  reg_file[R1] = 0;
  reg_file[R2] = 0;
  ASSERT_TRUE(execute());
  ASSERT_EQ(0, prog_mem[R0]);
}

TEST(LogicalTest, OR) {
  set_operands(R0, R1, R2);
  set_operation(AND);

  // TRUE && TRUE
  reg_file[R0] = 100;
  reg_file[R1] = 1;
  reg_file[R2] = 1;
  ASSERT_TRUE(execute());
  ASSERT_EQ(1, prog_mem[R0]);

  // TRUE && TRUE
  reg_file[R0] = 100;
  reg_file[R1] = 5234234;
  reg_file[R2] = 2083423;
  ASSERT_TRUE(execute());
  ASSERT_EQ(1, prog_mem[R0]);

  // FALSE && TRUE
  reg_file[R0] = 100;
  reg_file[R1] = 0;
  reg_file[R2] = 2083423;
  ASSERT_TRUE(execute());
  ASSERT_EQ(1, prog_mem[R0]);

  // FALSE && FALSE
  reg_file[R0] = 100;
  reg_file[R1] = 0;
  reg_file[R2] = 0;
  ASSERT_TRUE(execute());
  ASSERT_EQ(0, prog_mem[R0]);
}
