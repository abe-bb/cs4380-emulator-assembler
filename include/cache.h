#pragma once

class Cache {
  public:
    Cache(unsigned char* prog_mem);

    virtual unsigned int readByte(unsigned int address, unsigned char& outByte);
    virtual unsigned int writeByte(unsigned int address, unsigned char byte);
    virtual unsigned int readWord(unsigned int address, unsigned int& outWord);
    virtual unsigned int writeWord(unsigned int address, unsigned int word);

  private:
    unsigned char* prog_mem;
};
