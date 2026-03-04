import re
import sys

from asm_types import AssemblerError, usage_error, assembler_error, AsmState, AsmLine
from states import LineStart, LineEnd

if len(sys.argv) != 2:
    usage_error()

in_path = sys.argv[1]
out_path = re.sub("asm$", "bin", in_path)

state = LineStart()
line_num = -1

def labels_to_addresses(asm_state: AsmState, last_line: int):
    for label_marker in asm_state.label_list:
        # label not mapped to an address
        if not label_marker.label in asm_state.label_map:
            raise AssemblerError(last_line)

        address = asm_state.label_map[label_marker.label]
        addr_bytes = address.to_bytes(4, byteorder="little", signed=False)
        for i in range(4):
            asm_state.bytecode[i + label_marker.location] = addr_bytes[i]

try:
    with open(in_path, "r") as in_file:
        asm_state = AsmState()
        for line in in_file:
            line_num += 1
            asm_line = AsmLine(line[0:-1], line_num)

            state = LineStart()

            while not type(state) is LineEnd:
                next_state = state.run(asm_state, asm_line)
                state = next_state

        labels_to_addresses(asm_state, line_num)

        with open(out_path, "wb") as bin_file:
            bin_file.write(asm_state.bytecode)

except FileNotFoundError:
    usage_error()
except AssemblerError as e:
    assembler_error(e)