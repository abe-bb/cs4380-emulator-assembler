#include "../include/cache.h"
#include <climits>
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

    this->sets = std::vector<CacheSet>();

    // create CacheSets and CacheLines
    unsigned int num_sets = cache_lines / associativitiy;
    unsigned int block_bits = 0;
    unsigned int set_bits = 0;

    unsigned int v = block_size;
    while (v >> 1 <= 1) {
      block_bits += 1;
    }

    v = num_sets;
    while (v >> 1 <= 1) {
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
    }
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

   for (CacheSet& set : sets) {
   }
}

unsigned int NWayCache::writeWord(unsigned int address, unsigned int word) {
  *(unsigned int*)(prog_mem + address) = word;
  return 8;
}

unsigned int NWayCache::get_set_id(unsigned int address) {
  return (address & set_mask) >> block_bits;
}

unsigned char NWayCache::split_bits(unsigned int address, unsigned int& outSecondAddr) {
  unsigned int block_address = address & block_mask;
  
  
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

unsigned int CacheSet::readWord(unsigned int tag, unsigned int bo, unsigned int& outWord) {
  counter += 1;

  unsigned long lru_num = ULONG_MAX;
  CacheLine& lru_line = lines.front();
  for (CacheLine& line : lines) {
    // found uninitialized block
    if (!line.isValid()) {
      line.load_block(prog_mem, tag, set);
      outWord = line.readWord(bo, counter);
      return 0;
    }

    // found matching block
    if (line.get_tag() == tag) {
      outWord = line.readWord(bo, counter);
      return 0;
    }

    // track lru cache line
    if (line.get_used() < lru_num) {
      lru_line = line;
      lru_num = line.get_used();
    }
  }

  // if we get here, the tag did not match any cached blocks
  lru_line.write_block(prog_mem, set);
  lru_line.load_block(prog_mem, tag, set);

  outWord = lru_line.readWord(bo, counter);
  return 0;
}

unsigned int CacheSet::writeWord(unsigned int tag, unsigned int bo, unsigned int word) {
  counter += 1;

  unsigned long lru_num = ULONG_MAX;
  CacheLine& lru_line = lines.front();
  for (CacheLine& line : lines) {
    // found uninitialized block
    if (!line.isValid()) {
      line.load_block(prog_mem, tag, set);
      line.writeWord(bo, word, counter);
      return 0;
    }

    // found matching block
    if (line.get_tag() == tag) {
      line.writeWord(bo, word, counter);
      return 0;
    }

    // track lru cache line
    if (line.get_used() < lru_num) {
      lru_line = line;
      lru_num = line.get_used();
    }
  }

  // if we get here, the tag did not match any cached blocks
  lru_line.write_block(prog_mem, set);
  lru_line.load_block(prog_mem, tag, set);

  lru_line.writeWord(bo, word, counter);
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

void CacheLine::writeWord(unsigned int block_offset, unsigned int word, unsigned long used) {
  changed = true;
  this->used = used;

  if (block_offset + 3 < block.size()) {
    block[block_offset + 3] = (word >> 24) & 0xFF;
  }
  if (block_offset + 2 < block.size()) {
    block[block_offset + 2] = (word >> 16) & 0xFF;
  }
  if (block_offset + 1 < block.size()) {
    block[block_offset + 1] = (word >> 8) & 0xFF;
  }
  if (block_offset < block.size()) {
    block[block_offset] = word & 0xFF;
  }
  return;
}

void CacheLine::write_block(unsigned char* prog_mem, unsigned int set) {
  // if nothing changed no need to write back 
  if (!changed) {
    return;
  }

  unsigned int block_address = assemble_block_address(tag, set);

  for (auto i = 0; i < block.size(); i++) {
    prog_mem[block_address + i] = block[i];
  }
}

void CacheLine::load_block(unsigned char* prog_mem, unsigned int tag, unsigned int set) {
  valid = true;
  changed = false;

  this->tag = tag;  

  unsigned int block_address = assemble_block_address(tag, set);

  // read from memory into block
  for (auto i = 0; i < block.size(); i++) {
    block[i] = prog_mem[block_address + i];
  }
}

unsigned int CacheLine::assemble_block_address(unsigned int tag, unsigned int set) {
    return ((tag << (32 - tag_bits)) | (set << bo_bits));
}

