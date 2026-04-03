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
    // TODO: calculate timing
    return 8;
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

    // TODO: calculate timing
    return 8;
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

  unsigned int first_timing = 0;
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
   // TODO: Calculate timing
   return 8;
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

   // TODO: Calculate timing
   return 10;
  }

   throw std::runtime_error("Failed to find matching set in cache!!");
}

unsigned int NWayCache::writeWord(unsigned int address, unsigned int word) {
   // detect split block writes
   auto set_id = get_set_id(address);
   unsigned int second_addr = 0;
   unsigned int second_addr_bytes = split_blocks(address, second_addr);
   unsigned int first_addr_bytes = 4 - second_addr_bytes;

   unsigned int first_timing = 0;
   for (CacheSet& set : sets) {
     if (set.get_set() != set_id) {
       continue;
     }

     unsigned int tag = (address & tag_mask) >> (32 - tag_bits);
     first_timing = set.writeWord(tag, address & block_mask, word, first_addr_bytes);
     break;
   }
   
   if (second_addr_bytes == 0) {
     // TODO: Calculate timing
     return 8;
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

     // TODO: Calculate timing
     return 10;
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

unsigned int CacheSet::readWord(unsigned int tag, unsigned int bo, unsigned int& outWord) {
  counter += 1;

  unsigned long lru_num = ULONG_MAX;
  CacheLine* lru_line = &lines.front();
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
      lru_line = &line;
      lru_num = line.get_used();
    }
  }

  // if we get here, the tag did not match any cached blocks
  lru_line->write_block(prog_mem, set);
  lru_line->load_block(prog_mem, tag, set);

  outWord = lru_line->readWord(bo, counter);
  return 0;
}

unsigned int CacheSet::writeWord(unsigned int tag, unsigned int bo, unsigned int word, unsigned char num_bytes) {
  counter += 1;

  unsigned long lru_num = ULONG_MAX;
  CacheLine* lru_line = &lines.front();
  for (CacheLine& line : lines) {
    // found uninitialized block
    if (!line.isValid()) {
      line.load_block(prog_mem, tag, set);
      line.writeWord(bo, word, counter, num_bytes);
      return 0;
    }

    // found matching block
    if (line.get_tag() == tag) {
      line.writeWord(bo, word, counter, num_bytes);
      return 0;
    }

    // track lru cache line
    if (line.get_used() < lru_num) {
      lru_line = &line;
      lru_num = line.get_used();
    }
  }

  // if we get here, the tag did not match any cached blocks
  lru_line->write_block(prog_mem, set);
  lru_line->load_block(prog_mem, tag, set);

  lru_line->writeWord(bo, word, counter, num_bytes);
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

