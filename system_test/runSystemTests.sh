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

# TODO: convert hex files ot binary
mkdir -p binary


# Run tests
../build/emu4380 ./binary/exit
  echo -e "${GREEN}TEST: exit"
if [ $? = 0 ]; then 
  echo -e "RESULT: passed${NONE}"
else 
  echo -e "${RED}RESULT: failed${NONE}"
fi
