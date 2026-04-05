#include <climits>
#include <fstream>
#include <ios>
#include <iostream>
#include <iterator>
#include <string>
#include <vector>
#include "../include/emu4380.h"

enum CacheType {
    NoCache = 0,
    DirectMapped = 1,
    FullyAssociative = 2,
    SetAssociative2Way = 3,
    Custom = 4,
};

class Args {
    public:
        std::string bin_file;
        unsigned int mem_size;
        CacheType cache_type;

        unsigned int block_size;
        unsigned int cache_lines;
        unsigned int associativity;

        Args(std::string bin_file, unsigned int mem_size, CacheType cache_type,
             unsigned int block_size, unsigned int cache_lines, unsigned int associativity) :
             bin_file(bin_file), mem_size(mem_size), cache_type(cache_type), block_size(block_size),
             cache_lines(cache_lines), associativity(associativity) {}
};

void setup_memory(unsigned int mem_size, std::vector<unsigned char> program) {
    if (program.size() > mem_size) {
        std::cout << "INSUFFICIENT MEMORY SPACE\n";
        std::cout << std::flush;
        exit(2);
    }

    init_mem(mem_size);

    // copy program to memory
    for (unsigned int i = 0; i < program.size(); i++) {
        prog_mem[i] = program[i];
    }

    // load first 4 bytes into PC register
    reg_file[PC] = *(unsigned int*)prog_mem;
}

void setup_cache(Args& args) {
    if (args.cache_type == Custom) {
        delete cache;
        cache = new NWayCache(prog_mem, args.block_size, args.cache_lines, args.associativity);
    }
    else {
        init_cache(args.cache_type);
    }
}

void emulator_error(unsigned int instruction_addr) {
    std::cout << "INVALID INSTRUCTION AT: " << instruction_addr << "\n";
    std::cout << std::flush;
}

void cleanup() {
    std::cout << "Execution completed. Total memory cycles: " << mem_cycle_cntr << "\n";
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

Args parse_args(int argc, char* argv[]) {

    if (argc < 2 || argc % 2 != 0) {
        std::cout << "USAGE emu4380 [-c <cache config>] [-m <memory size>] <binary file>\n";
        exit(4);
    }
    
    std::string bin_file = "";
    unsigned int mem_size = 0b1 << 17;
    CacheType cache_type = NoCache;

    unsigned int block_size = 16;
    unsigned int cache_lines = 64;
    unsigned int associativity = 1;
    
    // parse arguments
    for (int i = 1; i < argc; i++) {
        std::string arg = std::string(argv[i]);
        // parse cache type
        if (arg == "-c") {
            i++;
            if (i >= argc) {
                std::cout << "Invalid arguments\n";
                exit(4);
            }
            int c = std::stoi(argv[i]);
            if (c < 0 || c > 4) {
                std::cout << "Invalid arguments";
                exit(4);
            }
            cache_type = static_cast<CacheType>(c);
        }
        // parse memory size
        else if (arg == "-m") {
            i++;
            if (i >= argc) {
                std::cout << "Invalid arguments\n";
                exit(4);
            }
            unsigned long c = std::stoul(argv[i]);
            if (c > UINT_MAX) {
                std::cout << "Invalid arguments\n";
                exit(4);
            }
            mem_size = (unsigned int)c;
        }
        // parse memory size
        else if (arg == "-b") {
            i++;
            if (i >= argc) {
                std::cout << "Invalid arguments\n";
                exit(4);
            }
            unsigned long c = std::stoul(argv[i]);
            if (c > UINT_MAX) {
                std::cout << "Invalid arguments\n";
                exit(4);
            }
            block_size = (unsigned int)c;
        }
        // parse memory size
        else if (arg == "-l") {
            i++;
            if (i >= argc) {
                std::cout << "Invalid arguments\n";
                exit(4);
            }
            unsigned long c = std::stoul(argv[i]);
            if (c > UINT_MAX) {
                std::cout << "Invalid arguments\n";
                exit(4);
            }
            cache_lines = (unsigned int)c;
        }
        // parse memory size
        else if (arg == "-a") {
            i++;
            if (i >= argc) {
                std::cout << "Invalid arguments\n";
                exit(4);
            }
            unsigned long c = std::stoul(argv[i]);
            if (c > UINT_MAX) {
                std::cout << "Invalid arguments\n";
                exit(4);
            }
            associativity = (unsigned int)c;
        }
        else {
            bin_file = std::string(argv[i]);
        }
    }    

    return Args(bin_file, mem_size, cache_type, block_size, cache_lines, associativity);
}

int main(int argc, char* argv[]) {
    // parse arguments
    Args args = parse_args(argc, argv);
    // std::cout << "Cache Type: " << args.cache_type << "\n"
    //     << "Memory Size: " << args.mem_size << "\n";

    // read file in as bytes
    std::ifstream in_file(args.bin_file, std::ios_base::binary);

    auto begin = std::istreambuf_iterator<char>(in_file);
    auto end = std::istreambuf_iterator<char>();
    std::vector<unsigned char> program(begin, end);

    setup_memory(args.mem_size, program);
    setup_cache(args);

    return emulator_loop();
}
