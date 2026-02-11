#include <fstream>
#include <ios>
#include <iostream>
#include <iterator>
#include <vector>
#include "../include/emu4380.h"

void setup_memory(unsigned int mem_size, std::vector<unsigned char> program) {
    if (program.size() > mem_size) {
        std::cout << "INSUFFICIENT MEMORY SPACE\n";
        std::cout << std::flush;
        exit(2);
    }

    init_mem(mem_size);

    // copy program to memory. I would love to combine this step with
    // init_mem, but the spec says init_mem must initialze prog_mem
    // separately so I can't.
    for (unsigned int i = 0; i < program.size(); i++) {
        prog_mem[i] = program[i];
    }

    // load first 4 bytes into PC register
    reg_file[PC] = *(unsigned int*)prog_mem;
}

void emulator_error(unsigned int instruction_addr) {
    std::cout << "INVALID INSTRUCTION AT: " << instruction_addr << "\n";
    std::cout << std::flush;
}

void cleanup() {
    std::cout << std::flush;
}

int emulator_loop() {
    while (true) {
        unsigned int current_addr = reg_file[PC];

        if (!fetch()) {
            emulator_error(current_addr);
            return 1;
        }

        if (!decode()) {
            emulator_error(current_addr);
            return 1;
        }

        if (!execute()) {
            emulator_error(current_addr);
            return 1;
        }

        if (flag == TERMINATE) {
            cleanup();
            exit(0);
        }
    }
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cout << "A binary file argument is required\n";
        return 3;
    }

    // read file in as bytes
    std::string in_path(argv[1]);
    std::ifstream in_file(in_path, std::ios_base::binary);

    auto begin = std::istreambuf_iterator<char>(in_file);
    auto end = std::istreambuf_iterator<char>();
    std::vector<unsigned char> program(begin, end);

    // read in second argument as memory size
    unsigned int mem_size = 0b1 << 17;
    if (argc >= 3) {
        std::string in_mem_size = argv[2];

        unsigned int potential_mem_size = 0;
        if (!parse_unsigned_int(in_mem_size, potential_mem_size)) {
            std::cout << "Invalid memory size. Max memory size is 4294967295.\n" << std::flush;
            return 4;
        }
        mem_size = potential_mem_size;
    }

    setup_memory(mem_size, program);

    return emulator_loop();
}
