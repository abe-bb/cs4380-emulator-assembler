#include <cstdint>
#include <gtest/gtest.h>
#include <vector>
#include "../include/emu4380.h"

// helper function for initializing memory
void initialize_memory(unsigned int size = 131072) {
// not sure if MEM_SIZE needs to be set by me, but whatever
  MEM_SIZE = size;
  init_mem(MEM_SIZE);
}

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

// operations categorized by number of operands
std::vector<unsigned int> operations_0operand_3dc = {1, 31};
std::vector<unsigned int> operations_1operand_2dc = {8, 9, 10, 11, 12, 13};
std::vector<unsigned int> operations_2operand_1dc = {7, 19, 21, 23, 26};
std::vector<unsigned int> operations_3operand_0dc = {18, 20, 22, 24, 25};

TEST(Setup, TestMemoryInit) {
  initialize_memory();

  for(unsigned int i = 0; i < MEM_SIZE; i++) {
    ASSERT_EQ(prog_mem[i], 0);
  }
}

// out of bounds memory fetch should fail
TEST(Fetch, OutOfBoundsAddressFails) {
  initialize_memory();
  reg_file[PC] = MEM_SIZE + 1;
  ASSERT_FALSE(fetch());
}

// fetch from valid memory location should succeed
TEST(Fetch, ValidAddressSucceeds) {
  initialize_memory();

  // set the program counter to the 5th byte of memory
  reg_file[PC] = 4;

  ASSERT_TRUE(fetch());
}

// validate that the bytes placed in the contrrol registers are correct (and in correct little endian order)
TEST(Fetch, BytesPlacedInCtrlRegs) {
  initialize_memory(10000);

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

// tests that fetch properly increments PC
TEST(Fetch, IncrementsPC) {
  init_mem(1024);
  reg_file[PC] = 0;
  ASSERT_TRUE(fetch());
  EXPECT_EQ(reg_file[PC], 8);
}

TEST(Decode, DontCareValuesShouldntFail) {

  for (auto operation : operations_2operand_1dc) {
    for (unsigned int invalid_operand = 16; invalid_operand < 256; invalid_operand++) {
      set_operands(operation);
      set_operands(R0, R1, invalid_operand);

      ASSERT_TRUE(decode()) << "Don't Care register value caused decode failure";
    }
  }

  for (auto operation : operations_1operand_2dc) {
    for (unsigned int invalid_operand = 16; invalid_operand < 256; invalid_operand++) {
      set_operands(operation);
      set_operands(R0, invalid_operand, invalid_operand);

      ASSERT_TRUE(decode()) << "Don't Care register value caused decode failure";
    }
  }

  for (auto operation : operations_3operand_0dc) {
    for (unsigned int invalid_operand = 16; invalid_operand < 256; invalid_operand++) {
      set_operands(operation);
      set_operands(invalid_operand, invalid_operand, invalid_operand);

      ASSERT_TRUE(decode()) << "Don't Care register value caused decode failure";
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
    for (unsigned int invalid_operand = 16; invalid_operand < 256; invalid_operand++) {
      set_operands(invalid_operand);

      EXPECT_FALSE(decode()) << "Operation: " << operation << " with all operands set to: " <<
      invalid_operand << "decode succeeded despite invalid operands";
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
  set_operands(R0, R1, R2);
  set_immediate(4);

  // list valid operator values
  std::vector<unsigned int> valid_operations = {1, 7, 8, 9, 10, 11, 12, 13, 18, 19, 20, 21, 22, 23, 24, 25, 26, 31};

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
  std::vector<unsigned int> invalid_operations = {0, 2, 3, 4, 5, 6, 14, 15, 16, 17, 27, 28, 29, 30, 32, 33, 55, 100, 200, 255};

  // loop over all valid operators and decode them
  for (unsigned int i : invalid_operations) {
    // set the OPERATION control register
    cntrl_regs[OPERATION] = i;

    EXPECT_FALSE(decode()) << "decode for operation: " << i << "returned success (invalid operation)";
  }
}

TEST(ExecuteJump, JumpSetsPC) {
  initialize_memory(1024);
  set_operation(JMP);
  set_immediate(72);

  EXPECT_TRUE(execute());
  EXPECT_EQ(72, reg_file[PC]);
}

TEST(ExecuteJump, JumpBeyondMemoryFails) {
  initialize_memory(1024);
  set_operands(JMP);
  set_immediate(2048);

  EXPECT_FALSE(execute());
  
}

TEST(ExecuteMem, MovCopiesContents) {
  set_operation(MOV);
  set_operands(R1, R2);
  reg_file[R2] = 0x0FF1CE;

  EXPECT_TRUE(execute());
  ASSERT_EQ(0x0FF1CE, reg_file[R1]);

}

TEST(ExecuteMem, MoviPutsImmediateInRegister) {
  initialize_memory(1024);
  set_operation(MOVI);
  set_operands(R5);
  set_immediate(0x45FA78ED);

  reg_file[R5] = 0x00;

  EXPECT_TRUE(execute());

  ASSERT_EQ(0x45FA78ED, reg_file[R5]) << "MOVI did not store immediate value in the specified register";
}

TEST(ExecuteMem, LdaLoadsMemoryToRegister) {
  initialize_memory(1024);
  set_operation(LDA);
  set_operands(R6);
  set_immediate(4);

  prog_mem[4] = 0xAF;

  EXPECT_TRUE(execute());

  ASSERT_EQ(0xAF, reg_file[R6]) << "LDA failed to load memory contents to register";
}

// loads the value from memory into a register
TEST(ExecuteMem, TestLDA) {
  init_mem(1024);

  prog_mem[60] = 0x21;
  prog_mem[61] = 0x43;
  prog_mem[62] = 0x65;
  prog_mem[63] = 0x87;

  set_operation(LDA);
  set_operands(R0);
  set_immediate(60);

  ASSERT_TRUE(execute());
  EXPECT_EQ(0x87654321, reg_file[R0]);
}

TEST(ExecuteMem, StrStoresIntToMemory) {
  initialize_memory(1024);
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

TEST(ExecuteMem, TestLDR) {
  init_mem(1024);

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

TEST(ExecuteMem, TestSTB) {
  init_mem(1024);

  reg_file[R0] = 0xCD;

  set_operation(STB);
  set_operands(R0);
  set_immediate(60);

  ASSERT_TRUE(execute());
  // check 0x87654321 interpreted as two's compliment
  EXPECT_EQ(0xCD, prog_mem[60]);
}


TEST(ExecuteMem, TestLDB) {
  init_mem(1024);

  prog_mem[60] = 0xCD;

  set_operation(LDB);
  set_operands(R0);
  set_immediate(60);

  ASSERT_TRUE(execute());
  // check 0x87654321 interpreted as two's compliment
  EXPECT_EQ(prog_mem[60], 0xCD);
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

  reg_file[R0] = 51;
  reg_file[R1] = 34;

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

TEST(ExecuteMath, TestDIV) {
  set_operation(DIV);
  set_operands(R2, R1, R0);
  set_immediate(0);

  reg_file[R1] = 109568;
  reg_file[R0] = 428;

  ASSERT_TRUE(execute());

  ASSERT_EQ(256, reg_file[R2]);
}

TEST(ExecuteMath, TestSDIV) {
  set_operation(SDIV);
  set_operands(R2, R1, R0);
  set_immediate(0);

  reg_file[R1] = -109568;
  reg_file[R0] = 428;

  ASSERT_TRUE(execute());

  ASSERT_EQ(-256, (int)reg_file[R2]);
}

TEST(ExecuteMath, TestDIVI) {
  set_operation(DIVI);
  set_operands(R2, R1);
  set_immediate(428);

  reg_file[R1] = 109568;

  ASSERT_TRUE(execute());

  ASSERT_EQ(256, reg_file[R2]);
}

