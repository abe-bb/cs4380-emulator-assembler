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
R0\t1
R1\t2
R2\t4
R3\t8
R4\t16
R5\t32
R6\t64
R7\t128
R8\t256
R9\t512
R10\t1024
R11\t2048
R12\t4096
R13\t8192
R14\t16384
R15\t32768
PC\t192
SL\t131072
SB\t262144
SP\t524288
FP\t1048576
HP\t2097152
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
