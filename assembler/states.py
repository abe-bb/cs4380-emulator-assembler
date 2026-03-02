# State Flow
#
# LineStart to Label | Directive | Instruction | LineEnd
# Label to Directive | Instruction
# Instruction | Directive to LineEnd
#
# All of them can transition to Error



from asm_types import AsmState, AsmLine, AssemblerError, Stage


class State:
    def run(self, asm_state: AsmState, line: AsmLine) -> State:
        pass

class LineStart(State):
    def run(self, asm_state: AsmState, line: AsmLine):
        assert(line.index == 0)

        # check for empty or comment line
        if len(line.line) == 0 or line.line[line.index] == ";":
            return LineEnd

        # check for label
        if line.line[line.index].isalnum():
            return Label

        # throw error for non space or tab character
        if not (line.line[line.index] == " " or line.line[line.index] == "\t"):
            raise AssemblerError(line.line_num)

        line.index += 1
        # loop over whitespace looking for directives, instructions, comments, or end of line
        while True:
            # end of line
            if len(line.line) <= line.index:
                return LineEnd
            # comment, so end the line
            if line.line[line.index] == ";":
                return LineEnd
            # alphabetic, so instruction
            if line.line[line.index].isalpha():
                asm_state.stage = Stage.Code
                return Instruction
            # period, so directive
            if line.line[line.index] == ".":
                # Directives only allowed in the data stage
                if asm_state.stage != Stage.Data:
                    raise AssemblerError(line.line_num)

                return Directive
            # space or tab are okay, just move to the next character
            if line.line[line.index] == " " or line.line[line.index] == "\t":
                line.index += 1
                continue

            # no valid character found, so throw an error
            raise AssemblerError(line.line_num)

class Instruction(State):
    def run(self, asm_state: AsmState, line: AsmLine) -> State:
        pass

class Directive(State):
    def run(self, asm_state: AsmState, line: AsmLine) -> State:
        pass

class Label(State):
    def run(self, asm_state: AsmState, line: AsmLine):
        label_name = ""

        # store the label
        while line.line[line.index].isalnum() or line.line[line.index] == "$" or line.line[line.index] == "_":
            label_name += line.line[line.index]

            # increment the index and throw an error if we reach the end of the line
            line.index += 1
            if line.index >= len(line.line):
                raise AssemblerError(line.line_num)

        # store the location of this label
        asm_state.label_map[label_name] = len(asm_state.bytecode)

        # loop over spaces and tabs
        while line.line[line.index] == " " or line.line[line.index] == "\t":
            line.index += 1
            if line.index >= len(line.line):
                raise AssemblerError(line.line_num)

        # only valid options are instruction or directive (if in the directive stage)
        if line.line[line.index] == ".":
            if asm_state.stage != Stage.Data:
                raise AssemblerError(line.line_num)
            return Directive
        if line.line[line.index].isalpha():
            asm_state.stage = Stage.Code
            return Instruction

        # didn't find a valid character so raise an error
        raise AssemblerError(line.line_num)


class LineEnd(State):
    pass

class Error(State):
    def run(self, asm_state: AsmState, line: AsmLine) -> State:
        raise AssemblerError(line.line_num)

