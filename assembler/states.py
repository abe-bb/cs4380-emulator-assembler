# State Flow
#
# LineStart to Label | Directive | Instruction | LineEnd
# Label to Directive | Instruction
# Instruction | Directive to LineEnd
#
# All of them can transition to Error
from plist import Data

from asm_types import AsmState, AsmLine, AssemblerError, Stage, OperandType, LabelMarker


def skip_space_tab(line: AsmLine, allow_line_end = False):
    while line.index < len(line.line):
        if line.line[line.index] == " " or line.line[line.index] == "\t":
            line.index += 1
            continue
        else:
            return

    if not allow_line_end and line.index >= len(line.line):
        raise AssemblerError(line.line_num)

def parse_label_name(line: AsmLine) -> str:
    label_name = ""

    # store the label
    while line.line[line.index].isalnum() or line.line[line.index] == "$" or line.line[line.index] == "_":
        label_name += line.line[line.index]

        # increment the index and end if we hit end of line
        line.index += 1
        if line.index >= len(line.line):
            return label_name
    return label_name

def parse_alpha(line: AsmLine) -> str:
    alpha = ""

    while line.index < len(line.line) and line.line[line.index].isalpha():
        alpha += line.line[line.index]
        line.index += 1

    return alpha

def parse_alphanumeric(line: AsmLine) -> str:
    alphanum = ""

    while line.index < len(line.line) and line.line[line.index].isalnum():
        alphanum += line.line[line.index]
        line.index += 1

    return alphanum

def parse_numeric(line:AsmLine) -> int:
    if line.line[line.index] != "#":
        raise AssemblerError(line.line_num)
    increment_index(line)

    numeric = ""
    if line.line[line.index] == "-":
        numeric += "-"
        line.index += 1

    while line.index < len(line.line) and line.line[line.index].isnumeric():
        numeric += line.line[line.index]
        line.index += 1

    return int(numeric)

def parse_char(line: AsmLine) -> int:
    if line.line[line.index] != "'":
        raise AssemblerError(line.line_num)
    increment_index(line)

    char_byte = 0
    # character to byte
    if line.line[line.index] == "\\":
        increment_index(line)
        if not line.line[line.index] in escape_chars:
            raise AssemblerError(line.line_num)
        char_byte = ord(escape_chars[line.line[line.index]])
    else:
        char_byte = ord(line.line[line.index])
        if char_byte < 0 or char_byte > 255:
            raise AssemblerError(line.line_num)

    increment_index(line)
    if line.line[line.index] != "'":
        raise AssemblerError(line.line_num)
    increment_index(line, allow_eol=True)

    return char_byte

def handle_immediate(asm_state: AsmState, line: AsmLine):
    if line.line[line.index] == "#":
        numeric = parse_numeric(line)
        num_bytes = numeric.to_bytes(4, byteorder="little", signed=True)
        for i in range(4):
            asm_state.bytecode.append(num_bytes[i])
    elif line.line[line.index] == "'":
        char_byte = parse_char(line)
        asm_state.bytecode.append(char_byte)
        for i in range(3):
            asm_state.bytecode.append(0)
    else:
        label = parse_label_name(line)
        label_marker = LabelMarker(label, len(asm_state.bytecode))
        asm_state.label_list.append(label_marker)

        # add placeholder bytes
        for i in range(4):
            asm_state.bytecode.append(0)

def switch_to_code_stage(asm_state: AsmState):
    asm_state.stage = Stage.Code

    # write address to first 4 bytes
    inst_addr = len(asm_state.bytecode)
    addr_bytes = inst_addr.to_bytes(4, byteorder="little", signed=False);
    for i in range(4):
        asm_state.bytecode[i] = addr_bytes[i]

def increment_index(line: AsmLine, allow_eol=False):
    line.index += 1

    if not allow_eol and line.index >= len(line.line):
        raise AssemblerError(line.line_num)

# define required components per instruction
reg2 = [OperandType.Register, OperandType.Register, OperandType.DC, OperandType.DC_I, 1]
reg3 = [OperandType.Register, OperandType.Register, OperandType.Register, OperandType.DC_I, 2]
reg1_immed = [OperandType.Register, OperandType.DC, OperandType.DC, OperandType.Immediate, 1]
reg2_immed = [OperandType.Register, OperandType.Register, OperandType.DC, OperandType.Immediate, 2]
immed = [OperandType.DC, OperandType.DC, OperandType.DC, OperandType.Immediate, 0]
inst_operands = {
    "JMP": immed,
    "MOV": reg2,
    "MOVI": reg1_immed,
    "LDA": reg1_immed,
    "STR": reg1_immed,
    "LDR": reg1_immed,
    "STB": reg1_immed,
    "LDB": reg1_immed,
    "ADD": reg3,
    "ADDI": reg2_immed,
    "SUB": reg3,
    "SUBI": reg2_immed,
    "MUL": reg3,
    "MULI": reg2_immed,
    "DIV": reg3,
    "SDIV": reg3,
    "DIVI": reg2_immed,
    "TRP": immed
}

valid_registers = {"R0", "R1", "R2", "R3", "R4", "R5", "R6", "R7", "R8", "R9", "R10", "R11", "R12", "R13", "R14", "R15",
                   "PC", "SL", "SB", "SP", "FP", "HP"}

# define binary representations
bin_rep = {
    # instructions
    "JMP": 1, "MOV": 7, "MOVI": 8, "LDA": 9, "STR": 10, "LDR": 11, "STB": 12, "LDB": 13, "ADD": 18, "ADDI": 19,
    "SUB": 20, "SUBI": 21, "MUL": 22, "MULI": 23, "DIV": 24, "SDIV": 25, "DIVI": 26, "TRP": 31,
    # registers
    "R0": 0, "R1": 1, "R2": 2, "R3": 3, "R4": 4, "R5": 5, "R6": 6, "R7": 7, "R8": 8, "R9": 9, "R10": 10, "R11": 11,
    "R12": 12, "R13": 13, "R14": 14, "R15": 15, "PC": 16, "SL": 17, "SB": 18, "SP": 19, "FP": 20, "HP": 21
}

escape_chars = {"t": "\t",  "\\": "\\", "n": "\n", "'": "'", "\"": "\"", "r": "\r", "b": "\b"}

class State:
    def run(self, asm_state: AsmState, line: AsmLine) -> State:
        pass

class LineStart(State):
    def run(self, asm_state: AsmState, line: AsmLine):
        assert(line.index == 0)

        # check for empty or comment line
        if len(line.line) == 0 or line.line[line.index] == ";":
            return LineEnd()

        # check for label
        if line.line[line.index].isalnum():
            return Label()

        # throw error for non space or tab character
        if not (line.line[line.index] == " " or line.line[line.index] == "\t"):
            raise AssemblerError(line.line_num)
        increment_index(line)

        skip_space_tab(line, True)

        # comment or end of line, so end the line
        if line.index >= len(line.line) or line.line[line.index] == ";":
            return LineEnd()
        # alphabetic, so instruction
        if line.line[line.index].isalpha():
            # switch to Code stage if necessary
            if asm_state.stage == Stage.Data:
                switch_to_code_stage(asm_state)
            return Instruction()
        # period, so directive
        if line.line[line.index] == ".":
            # Directives only allowed in the data stage
            if asm_state.stage != Stage.Data:
                raise AssemblerError(line.line_num)
            return Directive()

        # no valid character found so raise an error
        raise AssemblerError(line.line_num)

class Instruction(State):
    def run(self, asm_state: AsmState, line: AsmLine):
        instruction = parse_alpha(line).upper()

        # invalid instruction found
        if not instruction in inst_operands:
            return Error()

        skip_space_tab(line)

        # store binary representation of the instruction
        asm_state.bytecode.append(bin_rep[instruction])

        # convert operands into their binary representations
        for i in range(4):
            operand = inst_operands[instruction][i]
            num_commas = inst_operands[instruction][4]
            # emit zeros for don't care positions
            if operand == OperandType.DC:
                asm_state.bytecode.append(0)
            elif operand == OperandType.DC_I:
                for j in range(4):
                    asm_state.bytecode.append(0)
            elif operand == OperandType.Register:
                skip_space_tab(line)
                register = parse_alphanumeric(line).upper()
                # error if invalid register
                if not register in valid_registers:
                    return Error()
                asm_state.bytecode.append(bin_rep[register])

                # check for comma and skip over it
                if i < num_commas:
                    skip_space_tab(line)
                    if line.line[line.index] != ",":
                        return Error()
                    increment_index(line)

            elif operand == OperandType.Immediate:
                skip_space_tab(line)
                handle_immediate(asm_state, line)

        # make sure there's nothing but whitespace and comments at the end of the line
        skip_space_tab(line, allow_line_end=True)
        if line.index < len(line.line) and line.line[line.index] != ";":
            return Error()

        # end the line
        return LineEnd()

class Directive(State):
    def run(self, asm_state: AsmState, line: AsmLine):
        assert line.line[line.index] == "."
        increment_index(line)

        dir_type = parse_alpha(line).upper()

        skip_space_tab(line, allow_line_end=True)
        if dir_type == "INT":
            integer = 0
            # check for missing operand
            if line.index < len(line.line) and line.line[line.index] != ";":
                if line.line[line.index] != "#":
                    return Error()
                integer = parse_numeric(line)

            # out of range
            if integer < -2147483648 or integer > 2147483647:
                return Error()
            # store bytes
            int_bytes = integer.to_bytes(4, byteorder="little", signed=True)
            for i in range(4):
                asm_state.bytecode.append(int_bytes[i])
        elif dir_type == "BYT":
            if line.line[line.index] != "#" and line.line[line.index] != "'":
                return Error()

            if line.line[line.index] == "#":
                integer = parse_numeric(line)
                # out of range
                if integer < 0 or integer > 255:
                    return Error()
                # store bytes
                int_byte = integer.to_bytes(1)
                asm_state.bytecode.append(int_byte[0])
            elif line.line[line.index] == "'":
                char_byte = parse_char(line)
                asm_state.bytecode.append(char_byte)


        # make sure there's nothing but whitespace and comments at the end of the line
        skip_space_tab(line, allow_line_end=True)
        if line.index < len(line.line) and line.line[line.index] != ";":
            return Error()

        return LineEnd()




class Label(State):
    def run(self, asm_state: AsmState, line: AsmLine):
        label_name = parse_label_name(line)

        # store the location of this label
        asm_state.label_map[label_name] = len(asm_state.bytecode)

        skip_space_tab(line)

        # only valid options are instruction or directive (if in the directive stage)
        if line.line[line.index] == ".":
            if asm_state.stage != Stage.Data:
                raise AssemblerError(line.line_num)
            return Directive()
        if line.line[line.index].isalpha():
            if asm_state.stage == Stage.Data:
                switch_to_code_stage(asm_state)
            return Instruction()

        # didn't find a valid character so raise an error
        raise AssemblerError(line.line_num)

class LineEnd(State):
    pass

class Error(State):
    def run(self, asm_state: AsmState, line: AsmLine) -> State:
        raise AssemblerError(line.line_num)
