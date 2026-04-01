#include "../include/cache.h"
#include <stdexcept>
#include <vector>

// Cache base class
Cache::Cache(unsigned char* prog_mem) {
  this->prog_mem = prog_mem;
}

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

}

unsigned int NWayCache::readByte(unsigned int address, unsigned char& outByte) {
  outByte = prog_mem[address];
  return 8;
}

unsigned int NWayCache::writeByte(unsigned int address, unsigned char byte) {
  prog_mem[address] = byte;
  return 8;
}

unsigned int NWayCache::readWord(unsigned int address, unsigned int& outWord) {
   outWord = *(unsigned int*)(prog_mem + address);
   return 8;
}

unsigned int NWayCache::writeWord(unsigned int address, unsigned int word) {
  *(unsigned int*)(prog_mem + address) = word;
  return 8;
}


// CacheSet
CacheSet::CacheSet(unsigned int set, std::vector<CacheLine> lines) {
  this->set = set;
  this->lines = lines;
}

unsigned int CacheSet::readWord(unsigned int tag, unsigned int set, unsigned int bo, unsigned int& outWord) {
  return 0;
}

unsigned int CacheSet::writeWord(unsigned int tag, unsigned int set, unsigned int bo, unsigned int word) {
  return 0;
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


unsigned int CacheLine::readWord(unsigned int block_address, unsigned int used) {
  unsigned int word = 0;
  if (block_address + 3 < block.size()) {
    word += block[block_address + 3] << 24;
  }
  if (block_address + 2 < block.size()) {
    word += block[block_address + 2] << 16;
  }
  if (block_address + 1 < block.size()) {
    word += block[block_address + 1] << 8;
  }
  if (block_address < block.size()) {
    word += block[block_address];
  }
  return word;
}

void CacheLine::writeWord(unsigned int block_address, unsigned int word, unsigned int used) {
  changed = true;

  if (block_address + 3 < block.size()) {
    block[block_address + 3] = (word >> 24) & 0xFF;
  }
  if (block_address + 2 < block.size()) {
    block[block_address + 2] = (word >> 16) & 0xFF;
  }
  if (block_address + 1 < block.size()) {
    block[block_address + 1] = (word >> 8) & 0xFF;
  }
  if (block_address < block.size()) {
    block[block_address] = word & 0xFF;
  }
  return;
}

void CacheLine::write_block(unsigned char* prog_mem, unsigned int set) {
  // if nothing changed no need to write back 
  if (!changed) {
    return;
  }

  unsigned int set_bits = 32 - (tag_bits + bo_bits);
  unsigned int block_address = ((tag << tag_bits) | (set << set_bits)) << bo_bits;

  for (auto i = 0; i < block.size(); i++) {
    prog_mem[block_address + i] = block[i];
  }
}

void CacheLine::load_block(unsigned char* prog_mem, unsigned int tag, unsigned int set) {
  valid = true;
  changed = false;

  this->tag = tag;  

  unsigned int set_bits = 32 - (tag_bits + bo_bits);
  unsigned int block_address = ((tag << tag_bits) | (set << set_bits)) << bo_bits;

  // read from memory into block
  for (auto i = 0; i < block.size(); i++) {
    block[i] = prog_mem[i];
  }
}

