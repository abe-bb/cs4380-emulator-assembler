import sys
from enum import Enum
from typing import NoReturn


class AssemblerError(Exception):
    def __init__(self, line_num: int):
        self.lineNum = line_num

class LabelMarker:
    def __init__(self, label: str, location: int):
        self.label = label
        self.location = location

class Stage(Enum):
    Data = 0
    Code = 1

class AsmLine:
    def __init__(self, line: str, line_num: int):
        self.line: str = line
        self.line_num: int = line_num
        self.index: int = 0

class AsmState:
    def __init__(self):
        self.bytecode = bytearray()
        self.label_map: dict[str, int] = dict()
        self.label_list = list[LabelMarker]

        self.stage = Stage.Data
        

def usage_error() -> NoReturn:
    print("USAGE: python3 asm4380.py inputFile.asm")
    sys.exit(1)

def assembler_error(e: AssemblerError):
    print("Assembler error encountered on line " + str(e.lineNum) + "!")
    sys.exit(2)