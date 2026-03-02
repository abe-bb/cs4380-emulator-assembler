import sys

from asm_types import AssemblerError, usage_error, assembler_error, AsmState

if len(sys.argv) != 2:
    usage_error()

in_path = sys.argv[1]

state = AsmState()

try:
    with open(in_path, "r") as in_file:
        raise AssemblerError(100)
except FileNotFoundError:
    usage_error()
except AssemblerError as e:
    assembler_error(e)