import re
import sys

from asm_types import AssemblerError, usage_error, assembler_error, AsmState, AsmLine
from states import LineStart, LineEnd

if len(sys.argv) != 2:
    usage_error()

in_path = sys.argv[1]
out_path = re.sub("asm$", "bin", in_path)

state = LineStart()
line_num = 0

try:
    with open(in_path, "r") as in_file:
        for line in in_file:
            asm_line = AsmLine(line[0:-1], line_num)
            asm_state = AsmState()

            while not type(state) is LineEnd:
                next_state = state.run(asm_state, asm_line)
                state = next_state

            line_num += 1

        # TODO: replace labels with addresses

        with open(out_path, "wb") as bin_file:
            bin_file.write(asm_state.bytecode)

except FileNotFoundError:
    usage_error()
except AssemblerError as e:
    assembler_error(e)