#pragma once

#include <vector>
class Cache {
  public:
    Cache(unsigned char* prog_mem);

    virtual unsigned int readByte(unsigned int address, unsigned char& outByte);
    virtual unsigned int writeByte(unsigned int address, unsigned char byte);
    virtual unsigned int readWord(unsigned int address, unsigned int& outWord);
    virtual unsigned int writeWord(unsigned int address, unsigned int word);

  protected:
    unsigned char* prog_mem;
};

class CacheLine {
  private:
    bool valid;
    bool changed;
    unsigned int tag;
    unsigned int used;

    unsigned char tag_bits;
    unsigned char bo_bits;

    std::vector<unsigned char> block;

  public:
    CacheLine(unsigned int block_size, unsigned char tag_bits, unsigned char bo_bits);

    unsigned int get_tag();
    unsigned int get_used();

    void write_block(unsigned char* prog_mem, unsigned int set);
    void load_block(unsigned char* prog_mem, unsigned int tag, unsigned int set);

    // This function reads a word from the cache.
    // This function WILL NOT read beyond the end of the block
    // bytes in the word beyond the end of the cache will be set to 0
    unsigned int readWord(unsigned int block_address, unsigned int used);

    // This function writes a word to the cache.
    // This function WILL NOT write beyond the end of the block
    // bytes in word byeond the end of the cache will be ignored
    void writeWord(unsigned int block_address, unsigned int word, unsigned int used);
};


class CacheSet {
  private:
    unsigned int set;
    std::vector<CacheLine> lines; 

  public:
    CacheSet(unsigned int set, unsigned char set_bits);
};


class NWayCache : Cache {
  public:
    // Constructor
    // cache_lines % associativity must equal 0
    // cache size = block_size * cache_lines
    // num sets = cache_lines / associativity
    // cache_lines must be a power of 2
    // block_size must be a power of 2
    // minimum block_size 4 bytes
    // minimum cache_lines 1
    NWayCache(unsigned char* prog_mem, unsigned int block_size, unsigned int cache_lines, unsigned int associativitiy);

    unsigned int readByte(unsigned int address, unsigned char &outByte) override;
    unsigned int writeByte(unsigned int address, unsigned char byte) override;
    unsigned int readWord(unsigned int address, unsigned int &outWord) override;
    unsigned int writeWord(unsigned int address, unsigned int word) override;

  private:
    std::vector<CacheSet> storage;
};
