#!/bin/bash
RED='\e[31m'
BLUE='\e[34m'
GREEN='\e[32m'
NONE='\e[0m'

# build emulator
cd ../build/
make

# return to testing directory
cd -

# convert all hex files to binary using xxd
mkdir -p binary
cd hex
for file in *; do
  xxd -r -c 8 "$file" "../binary/$file"
done
cd ..

# Run tests

# Test too large memory size
program_output="$(../build/emu4380 ./binary/trp1_prints_R3 4294967296 )"
exit_code=$?
echo -e "${GREEN}TEST: Memory argument too large"
if [ $exit_code -eq 4 ] && [ "$program_output" = "Invalid memory size. Max memory size is 4294967295." ]; then 
  echo -e "RESULT: passed${NONE}"
else 
  echo -e "${RED}RESULT: failed${NONE}"
fi

# Test memory negative
program_output="$(../build/emu4380 ./binary/trp1_prints_R3 -1 )"
exit_code=$?
echo -e "${GREEN}TEST: Memory is negative"
if [ $exit_code -eq 4 ] && [ "$program_output" = "Invalid memory size. Max memory size is 4294967295." ]; then 
  echo -e "RESULT: passed${NONE}"
else 
  echo -e "${RED}RESULT: failed${NONE}"
fi

# Test memory too small for program
program_output="$(../build/emu4380 ./binary/exit 8 )"
exit_code=$?
echo -e "${GREEN}TEST: Memory too small for program"
if [ $exit_code -eq 2 ] && [ "$program_output" = "INSUFFICIENT MEMORY SPACE" ]; then 
  echo -e "RESULT: passed${NONE}"
else 
  echo -e "${RED}RESULT: failed${NONE}"
fi


# Test TRP 0 
../build/emu4380 ./binary/exit
exit_code=$?
echo -e "${GREEN}TEST: TRP 0 exit"
if [ $exit_code -eq 0 ]; then 
  echo -e "RESULT: passed${NONE}"
else 
  echo -e "${RED}RESULT: failed${NONE}"
fi

# Test TRP 1 positive integer
program_output="$(../build/emu4380 ./binary/trp1_prints_positive_R3)"
exit_code=$?
echo -e "${GREEN}TEST: TRP 1 prints positive R3 to console"
if [ $exit_code -eq 0 ] && [ "$program_output" = "49923402" ]; then 
  echo -e "RESULT: passed${NONE}"
else 
  echo -e "${RED}RESULT: failed${NONE}"
fi

# Test TRP 1 negative integer
program_output="$(../build/emu4380 ./binary/trp1_prints_negative_R3)"
exit_code=$?
echo -e "${GREEN}TEST: TRP 1 prints negative R3 to console"
if [ $exit_code -eq 0 ] && [ "$program_output" = "-123934203" ]; then 
  echo -e "RESULT: passed${NONE}"
else 
  echo -e "${RED}RESULT: failed${NONE}"
fi

# Test TRP 2 reads integer
program_output="$(../build/emu4380 ./binary/trp2_reads_int <<< "-432890")"
exit_code=$?
echo -e "${GREEN}TEST: TRP 2 reads integer from console"
if [ $exit_code -eq 0 ] && [ "$program_output" = "-432890" ]; then 
  echo -e "RESULT: passed${NONE}"
else 
  echo -e "${RED}RESULT: failed${NONE}"
fi

# Test TRP 3 writes character
program_output="$(../build/emu4380 ./binary/trp3_writes_char)"
exit_code=$?
echo -e "${GREEN}TEST: TRP 3 writes character to console"
if [ $exit_code -eq 0 ] && [ "$program_output" = "H" ]; then 
  echo -e "RESULT: passed${NONE}"
else 
  echo -e "${RED}RESULT: failed${NONE}"
fi

# Test TRP 4 reads character
program_output="$(../build/emu4380 ./binary/trp4_reads_char <<< '!' )"
exit_code=$?
echo -e "${GREEN}TEST: TRP 4 reads character from console"
if [ $exit_code -eq 0 ] && [ "$program_output" = "!" ]; then 
  echo -e "RESULT: passed${NONE}"
else 
  echo -e "${RED}RESULT: failed${NONE}"
fi

# Test TRP 4 reads only a single character
program_output="$(../build/emu4380 ./binary/trp4_reads_char <<< '12345' )"
exit_code=$?
echo -e "${GREEN}TEST: TRP 4 reads only a single character from console"
if [ $exit_code -eq 0 ] && [ "$program_output" = "1" ]; then 
  echo -e "RESULT: passed${NONE}"
else 
  echo -e "${RED}RESULT: failed${NONE}"
fi

# Test TRP 98 prints register contents
expected_output=$(cat <<HEREDOC
Register Contents:
0\t| 0x1
1\t| 0x2
2\t| 0x4
3\t| 0x8
4\t| 0x10
5\t| 0x20
6\t| 0x40
7\t| 0x80
8\t| 0x100
9\t| 0x200
10\t| 0x400
11\t| 0x800
12\t| 0x1000
13\t| 0x2000
14\t| 0x4000
15\t| 0x8000
16\t| 0xc0
17\t| 0x20000
18\t| 0x40000
19\t| 0x80000
20\t| 0x100000
21\t| 0x200000
HEREDOC
)
# expand tabs 
printf -v expected_output "%b" "$expected_output"

program_output="$(../build/emu4380 ./binary/trp98)"
exit_code=$?
echo -e "${GREEN}TEST: TRP 98 prints all registers to console"
if [ $exit_code -eq 0 ] && [ "$program_output" = "$expected_output" ]; then 
  echo -e "RESULT: passed${NONE}"
else 
  echo -e "${RED}RESULT: failed${NONE}"
fi

export output="$program_output"
export expected="$expected_output"


# Test invalid operation in binary 
program_output="$(../build/emu4380 ./binary/invalid_operation)"
exit_code=$?
echo -e "${GREEN}TEST: invalid operation fails correctly"
if [ $exit_code -eq 1 ] && [ "$program_output" = "INVALID INSTRUCTION AT: 8" ]; then 
  echo -e "RESULT: passed${NONE}"
else 
  echo -e "${RED}RESULT: failed${NONE}"
fi
