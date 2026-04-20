#pragma once

#include <string>
#include <vector>

extern unsigned int MEM_SIZE;

extern unsigned int reg_file[22];
extern unsigned char* prog_mem;
extern unsigned int cntrl_regs[5];

extern unsigned int mem_cycle_cntr;

extern unsigned int cache_type;

class Cache {
  public:
    Cache(unsigned char* prog_mem);
    virtual ~Cache();

    virtual unsigned int readByte(unsigned int address, unsigned char& outByte);
    virtual unsigned int writeByte(unsigned int address, unsigned char byte);
    virtual unsigned int readWord(unsigned int address, unsigned int& outWord);
    virtual unsigned int writeWord(unsigned int address, unsigned int word);

    unsigned char* memory;
};

class CacheTime {
  public:
    unsigned int hits;
    unsigned int words_stored;
    unsigned int words_loaded;

    CacheTime();

    unsigned int calculate_timing(); 
    void add(CacheTime other);
};

class CacheLine {
  public:
    CacheLine(unsigned int block_size, unsigned char tag_bits, unsigned char bo_bits);

    unsigned int get_tag();
    unsigned int get_used();
    bool isValid();

    void write_block(unsigned char* prog_mem, unsigned int set, CacheTime& timing);
    void load_block(unsigned char* prog_mem, unsigned int tag, unsigned int set, CacheTime& timing);

    // This function reads a word from the cache.
    // This function WILL NOT read beyond the end of the block
    // bytes in the word beyond the end of the cache will be set to 0
    unsigned int readWord(unsigned int block_offset, unsigned long used);

    // This function writes a word to the cache.
    // This function WILL NOT write beyond the end of the block
    // bytes in word beyond the end of the cache will be ignored.
    // additionally, the num_bytes parameter can be set to a number < 4
    // to reduce the number of bytes written. 
    void writeWord(unsigned int block_offset, unsigned int word, unsigned long used, unsigned char num_bytes);

  private:
    bool valid;
    bool changed;
    unsigned int tag;
    unsigned long used;

    unsigned char tag_bits;
    unsigned char bo_bits;

    std::vector<unsigned char> block;

    unsigned int assemble_block_address(unsigned int tag, unsigned int set);
};


class CacheSet {
  public:
    CacheSet(unsigned int set, unsigned char tag_bits, unsigned char set_bits, std::vector<CacheLine> lines, unsigned char* prog_mem);

    unsigned int get_set();

    CacheTime readWord(unsigned int tag, unsigned int bo, unsigned int& outWord);
    CacheTime writeWord(unsigned int tag, unsigned int bo, unsigned int word, unsigned char num_bytes);

  private:
    unsigned int set;
    std::vector<CacheLine> lines; 

    unsigned char* prog_mem;

    unsigned char set_bits;
    unsigned char tag_bits;

    unsigned long counter;
};


class NWayCache : public Cache {
  public:
    // Constructor
    // cache size = block_size * cache_lines
    // num sets = cache_lines / associativity
    // 
    // cache_lines % associativity must equal 0
    // cache_lines must be a power of 2
    // block_size must be a power of 2
    // minimum block_size 4 bytes
    // minimum cache_lines 1
    NWayCache(unsigned char* prog_mem, unsigned int block_size, unsigned int cache_lines, unsigned int associativitiy);

    unsigned int readByte(unsigned int address, unsigned char &outByte) override;
    unsigned int writeByte(unsigned int address, unsigned char byte) override;
    unsigned int readWord(unsigned int address, unsigned int &outWord) override;
    unsigned int writeWord(unsigned int address, unsigned int word) override;

    // returns the set_id portion of the address
    unsigned int get_set_id(unsigned int address);
    // outSecondAddr will the address of the second block, returns the number
    // of bytes in the word that are in the second block
    unsigned char split_blocks(unsigned int address, unsigned int& outSecondAddr);
  private:
    std::vector<CacheSet> sets;

    unsigned char tag_bits;
    unsigned char set_bits;
    unsigned char block_bits;

    unsigned int tag_mask;
    unsigned int set_mask;
    unsigned int block_mask;  
};

extern Cache* cache;

enum RegNames { R0=0, R1, R2, R3, R4, R5, R6, R7, R8, R9, R10,
  R11, R12, R13, R14, R15, PC, SL, SB, SP, FP, HP };
enum CntrlRegNames{ OPERATION, OPERAND_1, OPERAND_2, OPERAND_3, IMMEDIATE };
enum Operations{JMP=1, JMR, BNZ, BGT, BLT, BRZ, MOV, MOVI, LDA,
  STR, LDR, STB, LDB, ISTR, ILDR, ISTB, ILDB, ADD, ADDI, SUB, SUBI,
  MUL, MULI, DIV, SDIV, DIVI, AND=27, OR, CMP=29, CMPI, TRP=31,
  ALCI=32, ALLC, IALLC, PSHR=35, PSHB, POPR, POPB, CALL, RET};

enum PostOpFlag {
  NOTHING = 0,
  TERMINATE
};

extern PostOpFlag flag;

bool init_mem(unsigned int size);
void init_cache(unsigned int cacheType);

bool fetch();
bool decode();
bool execute();

unsigned char readByte(unsigned int address);
unsigned int readWord(unsigned int address);
void writeByte(unsigned int address, unsigned char byte);
void writeWord(unsigned int address, unsigned int word);

// execute instruction functions
bool jmp();
bool mov();
bool movi();
bool lda();
bool str();
bool ldr();
bool stb();
bool ldb();
bool add();
bool addi();
bool sub();
bool subi();
bool mul();
bool muli();
bool div();
bool sdiv();
bool divi();
bool trp();

// convenience categorization of operations
extern std::vector<unsigned int> operations_0operand_3dc;
extern std::vector<unsigned int> operations_1operand_2dc;
extern std::vector<unsigned int> operations_2operand_1dc;
extern std::vector<unsigned int> operations_3operand_0dc;

// io functions
bool parse_unsigned_int(std::string input, unsigned int& output);
bool parse_int(std::string input, int& output);
