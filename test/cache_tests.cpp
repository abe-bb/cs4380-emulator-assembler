#include <cstdlib>
#include <gtest/gtest.h>

#include "../include/cache.h"

TEST(CacheDummy, ReadWriteByte) {
  unsigned int size = 1024;
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
  unsigned int size = 1024;
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
