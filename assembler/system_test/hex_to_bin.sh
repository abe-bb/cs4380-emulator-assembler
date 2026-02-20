#!/bin/bash


# convert all hex files to binary using xxd
mkdir -p expected
cd expected_hex
for file in *; do
  xxd -r "$file" "../expected/${file}.bin" 
done
cd ..
