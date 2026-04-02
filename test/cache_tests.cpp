#include <cstdlib>
#include <gtest/gtest.h>
#include <stdexcept>
#include <vector>

#include "../include/cache.h"

TEST(CacheDummy, ReadWriteByte) {
  unsigned int size = 128;
  unsigned char* prog_mem = new unsigned char[size];

  Cache cache = Cache(prog_mem);

  for (auto i = 0; i < size; i++) {
    unsigned char byte = rand();
    // check memory timing result
    ASSERT_EQ(8, cache.writeByte(i, byte));
    // check that dummy cache writes through
    ASSERT_EQ(byte, prog_mem[i]);

    unsigned char cache_byte;
    // check memory timing
    ASSERT_EQ(8, cache.readByte(i, cache_byte));
    // check that readByte returns correct value
    ASSERT_EQ(byte, cache_byte);
  }
  

  delete[] prog_mem;
}

TEST(CacheDummy, ReadWriteWord) {
  unsigned int size = 128;
  unsigned char* prog_mem = new unsigned char[size];

  Cache cache = Cache(prog_mem);

  for (auto i = 0; i < size - 3; i++) {
    unsigned int word = rand();
    // check memory timing result
    ASSERT_EQ(8, cache.writeWord(i, word));
    // check that dummy cache writes through
    ASSERT_EQ(word, *(unsigned int*)(prog_mem + i));

    unsigned int cache_word;
    // check memory timing
    ASSERT_EQ(8, cache.readWord(i, cache_word));
    // check that readByte returns correct value
    ASSERT_EQ(word, cache_word);
  }
  

  delete[] prog_mem;
}

TEST(CacheLine, Construction) {

  // valid cache line configuration
  ASSERT_NO_THROW(CacheLine(64, 18, 6)); 

  // bo_bits doesn't match block offset
  ASSERT_THROW(CacheLine(64, 18, 7), std::invalid_argument);

}

TEST(CacheLine, ReadsMemoryCorrectly) {
  CacheLine line = CacheLine(64, 26, 6);
  unsigned char* prog_mem = new unsigned char[128];

  // place values in memory
  for (auto i = 0; i < 128; i++) {
    prog_mem[i] = i * 2;
  }

  // read the first 64 bytes into the cache block
  line.load_block(prog_mem, 0, 0);

  for (auto i = 0; i < 64; i+= 4) {
    unsigned int cache_word = line.readWord(i, 0);

    ASSERT_EQ(i * 2, cache_word & 0xFF);
    ASSERT_EQ((i + 1) * 2, (cache_word >> 8) & 0xFF);
    ASSERT_EQ((i + 2) * 2, (cache_word >> 16) & 0xFF);
    ASSERT_EQ((i + 3) * 2, (cache_word >> 24) & 0xFF);
  }

  delete[] prog_mem;
}

TEST(CacheLine, WritesMemoryCorrectly) {
  CacheLine line = CacheLine(64, 26, 6);
  unsigned char* prog_mem = new unsigned char[128];
  // zero out memory
  for (auto i = 0; i < 128; i++) {
    prog_mem[i] = 0;
  }

  // read the first 64 bytes into the cache block
  line.load_block(prog_mem, 0, 0);

  // write values into the cache
  for (auto i = 0; i < 64; i += 4) {
    unsigned int word = 0;
    word += ((i + 3) * 2) << 24;
    word += ((i + 2) * 2) << 16;
    word += ((i + 1) * 2) << 8;
    word += i * 2;

    line.writeWord(i, word, 0);
  }

  // write values back to memory
  line.write_block(prog_mem, 0);

  // check values in memory
  for (auto i = 0; i < 64; i++) {
    ASSERT_EQ(i * 2, prog_mem[i]);
  }

  delete [] prog_mem;
}

TEST(CacheLine, SimpleWrite) {
  CacheLine line = CacheLine(64, 26, 6);
  unsigned char* prog_mem = new unsigned char[128];
  // zero out memory
  for (auto i = 0; i < 128; i++) {
    prog_mem[i] = 0;
  }

  // read the first 64 bytes into the cache block
  line.load_block(prog_mem, 0, 0);

  // write values into the cache
  line.writeWord(0, 0xAC, 0);

  // write values back to memory
  line.write_block(prog_mem, 0);

  // check values in memory
  ASSERT_EQ(0xAC, prog_mem[0]);

  delete [] prog_mem;
}

TEST(CacheSet, ReadsMemoryCorrectly) {
  CacheLine line1 = CacheLine(64, 26, 6);
  CacheLine line2 = CacheLine(64, 26, 6);
  std::vector<CacheLine> lines = {line1, line2};

  unsigned char* prog_mem = new unsigned char[128];

  CacheSet set = CacheSet(0, 26, 0, lines, prog_mem);


  // place values in memory
  for (auto i = 0; i < 128; i++) {
    prog_mem[i] = i * 2;
  }


  for (auto i = 0; i < 128; i+= 4) {
    unsigned int bo = i & 0x3F;
    unsigned int tag = i >> 6;
    unsigned int cache_word = 0;
    unsigned int mem_cycles = set.readWord(tag, bo, cache_word);

    ASSERT_EQ(i * 2, cache_word & 0xFF);
    ASSERT_EQ((i + 1) * 2, (cache_word >> 8) & 0xFF);
    ASSERT_EQ((i + 2) * 2, (cache_word >> 16) & 0xFF);
    ASSERT_EQ((i + 3) * 2, (cache_word >> 24) & 0xFF);
  }

  delete[] prog_mem;
}

TEST(CacheSet, WritesMemoryCorrectly) {
  CacheLine line1 = CacheLine(64, 26, 6);
  CacheLine line2 = CacheLine(64, 26, 6);
  std::vector<CacheLine> lines = {line1, line2};

  unsigned char* prog_mem = new unsigned char[256];

  CacheSet set = CacheSet(0, 26, 0, lines, prog_mem);
  // zero out memory
  for (auto i = 0; i < 256; i++) {
    prog_mem[i] = 0;
  }

  // write some memory in the first and second memory block
  set.writeWord(0, 0, 0xDEADBEEF);
  set.writeWord(1, 0, 0x89ABCDEF);


  // Read from 3rd and 4th memory block
  // (should flush first two from the cache,
  // causing changes to be written)
  unsigned int result;
  set.readWord(2, 0, result);
  set.readWord(3, 0, result);


  // first block
  ASSERT_EQ(0xEF, prog_mem[0]);
  ASSERT_EQ(0xBE, prog_mem[1]);
  ASSERT_EQ(0xAD, prog_mem[2]);
  ASSERT_EQ(0xDE, prog_mem[3]);

  // second block
  ASSERT_EQ(0xEF, prog_mem[64]);
  ASSERT_EQ(0xCD, prog_mem[65]);
  ASSERT_EQ(0xAB, prog_mem[66]);
  ASSERT_EQ(0x89, prog_mem[67]);

  delete [] prog_mem;
}
