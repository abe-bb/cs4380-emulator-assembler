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

# Test TRP 0 
../build/emu4380 ./binary/exit
exit_code=$?
echo -e "${GREEN}TEST: exit"
if [ $exit_code -eq 0 ]; then 
  echo -e "RESULT: passed${NONE}"
else 
  echo -e "${RED}RESULT: failed${NONE}"
fi

# Test invalid operation in binary 
program_output="$(../build/emu4380 ./binary/invalid_operation)"
exit_code=$?
echo -e "${GREEN}TEST: invalid operation fails correctly"
if [ $exit_code -eq 1 ] && [ "$program_output" = "INVALID INSTRUCTION AT: 8" ]; then 
  echo -e "RESULT: passed${NONE}"
else 
  echo -e "${RED}RESULT: failed${NONE}"
fi
