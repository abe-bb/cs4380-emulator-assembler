import sys
from typing import NoReturn

def usage_error() -> NoReturn:
    print("USAGE: python3 asm4380.py inputFile.asm")
    sys.exit(1)

class AssemblerError(Exception):
    def __init__(self, line_num: int):
        self.lineNum = line_num

def assembler_error(e: AssemblerError):
    print("Assembler error encountered on line " + str(e.lineNum) + "!")
    sys.exit(2)


if __name__ == "__main__":
    if len(sys.argv) != 2:
        usage_error()

    in_path = sys.argv[1]

    try:
        with open(in_path, "r") as in_file:
            raise AssemblerError(100)
    except FileNotFoundError:
        usage_error()
    except AssemblerError as e:
        assembler_error(e)