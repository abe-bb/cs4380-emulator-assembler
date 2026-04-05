#!/bin/bash
RED='\e[31m'
BLUE='\e[34m'
GREEN='\e[32m'
NONE='\e[0m'

# build emulator
cd ../build/ >/dev/null
make >/dev/null

# return to testing directory
cd - >/dev/null

# array of test program names
test_prog=("prog_a" "prog_b" "prog_c" "prog_d" "prog_e" "prog_f")

echo "Program, Cache Type, Timing"
for prog in "${test_prog[@]}"; do
  for cache_type in 0 1 2 3; do
    # echo "program: $prog  |  cache type: $cache_type"
    program_output="$(../build/emu4380 ./cache_testing/$prog.bin -c $cache_type)"
    mem_timing="$(echo $program_output | cut -c 43- -)"
    echo "$prog, $cache_type, $mem_timing"
  done
done
    



