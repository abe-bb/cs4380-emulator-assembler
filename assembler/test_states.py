import pytest

from asm_types import AsmState, AsmLine, Stage, AssemblerError
from states import Label, LineStart, Directive, Instruction, LineEnd

def test_start_to_label():
    asm_state = AsmState()
    line = AsmLine("HeLLo", 100)

    state = LineStart()
    result = state.run(asm_state, line)
    assert(type(result) is Label)

def test_start_to_directive():
    asm_state = AsmState()
    line = AsmLine(" \t \t \t.INT", 100)

    state = LineStart()
    result = state.run(asm_state, line)
    assert(type(result) is Directive)

def test_start_to_directive_in_code_fails():
    asm_state = AsmState()
    asm_state.stage = Stage.Code
    line = AsmLine(" \t \t \t.INT", 100)

    state = LineStart()
    with pytest.raises(AssemblerError) as errinfo:
        result = state.run(asm_state, line)
    assert errinfo.type is AssemblerError
    assert errinfo.value.lineNum == 100

def test_start_to_instruction():
    asm_state = AsmState()
    line = AsmLine(" \t \t \tTRP", 100)

    state = LineStart()
    assert(asm_state.stage == Stage.Data)
    result = state.run(asm_state, line)
    assert(type(result) is Instruction)
    assert(asm_state.stage == Stage.Code)

def test_start_to_comment():
    asm_state = AsmState()
    line = AsmLine(" \t \t \t;This is a comment", 100)

    state = LineStart()
    result = state.run(asm_state, line)
    assert(type(result) is LineEnd)

    line.line = ";This is a comment at the beginning"
    line.index = 0
    result = state.run(asm_state, line)
    assert (type(result) is LineEnd)

def test_start_empty_line():
    asm_state = AsmState()
    line = AsmLine("", 100)

    state = LineStart()
    result = state.run(asm_state, line)
    assert(type(result) is LineEnd)

def test_start_space_tab_line():
    asm_state = AsmState()
    line = AsmLine("\t\t\t\t                                                                    ", 100)

    state = LineStart()
    result = state.run(asm_state, line)
    assert(type(result) is LineEnd)

def test_label_invalid_character():
    asm_state = AsmState()
    line = AsmLine("HEL%LO .INT #56", 55)

    state = Label()
    with pytest.raises(AssemblerError) as errinfo:
        result = state.run(asm_state, line)
    assert (errinfo.type is AssemblerError)
    assert (errinfo.value.lineNum == 55)

def test_label_line_ends():
    asm_state = AsmState()
    line = AsmLine("HELLO \t\t\t   ", 55)

    state = Label()
    with pytest.raises(AssemblerError) as errinfo:
        result = state.run(asm_state, line)
    assert errinfo.type is AssemblerError
    assert errinfo.value.lineNum == 55

def test_label_to_directive():
    asm_state = AsmState()
    line = AsmLine("HELLO \t\t\t   .BYT 's'", 55)

    state = Label()
    result = state.run(asm_state, line)

    assert type(result) is Directive
    assert line.index == 12

def test_label_to_instruction():
    asm_state = AsmState()
    line = AsmLine("HELLO \t\t\t    TRP #0", 55)

    state = Label()
    result = state.run(asm_state, line)

    assert type(result) is Instruction
    assert line.index == 13

def test_instruction_trp_to_eol():
    asm_state = AsmState()

    state = Instruction()
    asm_line = AsmLine("  trp #10", 1)
    asm_line.index = 2

    result = state.run(asm_state, asm_line)
    assert type(result) is LineEnd
    assert asm_state.bytecode[-8] == 31
    assert asm_state.bytecode[-4] == 10

    asm_line.line = "  trp #98"
    asm_line.index = 2
    asm_line.line_num = 2
    result = state.run(asm_state, asm_line)
    assert type(result) is LineEnd
    assert asm_state.bytecode[-8] == 31
    assert asm_state.bytecode[-4] == 98

def test_instruction_jmp_to_eol():
    asm_state = AsmState()

    state = Instruction()
    asm_line = AsmLine("  JMP HeLLo", 1)
    asm_line.index = 2

    result = state.run(asm_state, asm_line)
    assert type(result) is LineEnd
    assert asm_state.bytecode[-8] == 1
    assert asm_state.bytecode[-7] == 0
    assert asm_state.bytecode[-6] == 0
    assert asm_state.bytecode[-5] == 0

    assert asm_state.label_list[0].label == "HeLLo"
    assert asm_state.label_list[0].location == (len(asm_state.bytecode) - 4)

def test_instruction_mov_to_eol():
    asm_state = AsmState()

    state = Instruction()
    asm_line = AsmLine("  mov R9 FP", 1)
    asm_line.index = 2

    result = state.run(asm_state, asm_line)
    assert type(result) is LineEnd
    assert asm_state.bytecode[-8] == 7
    assert asm_state.bytecode[-7] == 9
    assert asm_state.bytecode[-6] == 20
    assert asm_state.bytecode[-5] == 0
    assert asm_state.bytecode[-4] == 0
    assert asm_state.bytecode[-3] == 0
    assert asm_state.bytecode[-2] == 0
    assert asm_state.bytecode[-1] == 0

def test_instruction_movi_integer_to_eol():
    asm_state = AsmState()

    state = Instruction()
    asm_line = AsmLine("  movi R11 #2147483647", 1)
    asm_line.index = 2

    result = state.run(asm_state, asm_line)
    assert type(result) is LineEnd
    assert asm_state.bytecode[-8] == 8
    assert asm_state.bytecode[-7] == 11
    assert asm_state.bytecode[-6] == 0
    assert asm_state.bytecode[-5] == 0
    assert asm_state.bytecode[-4] == 255
    assert asm_state.bytecode[-3] == 255
    assert asm_state.bytecode[-2] == 255
    assert asm_state.bytecode[-1] == 127

def test_instruction_lda_address_to_eol():
    asm_state = AsmState()

    state = Instruction()
    asm_line = AsmLine(" \t lda R6 Th$$$is_is_a_label$", 1)
    asm_line.index = 3

    result = state.run(asm_state, asm_line)
    assert type(result) is LineEnd
    assert asm_state.bytecode[-8] == 9
    assert asm_state.bytecode[-7] == 6
    assert asm_state.bytecode[-6] == 0
    assert asm_state.bytecode[-5] == 0
    assert asm_state.bytecode[-4] == 0
    assert asm_state.bytecode[-3] == 0
    assert asm_state.bytecode[-2] == 0
    assert asm_state.bytecode[-1] == 0

    assert asm_state.label_list[0].label == "Th$$$is_is_a_label$"
    assert asm_state.label_list[0].location == 8

def test_instruction_add_to_eol():
    asm_state = AsmState()

    state = Instruction()
    asm_line = AsmLine(" \t adD r5 r8 r14", 1)
    asm_line.index = 3

    result = state.run(asm_state, asm_line)
    assert type(result) is LineEnd
    assert asm_state.bytecode[-8] == 18
    assert asm_state.bytecode[-7] == 5
    assert asm_state.bytecode[-6] == 8
    assert asm_state.bytecode[-5] == 14
    assert asm_state.bytecode[-4] == 0
    assert asm_state.bytecode[-3] == 0
    assert asm_state.bytecode[-2] == 0
    assert asm_state.bytecode[-1] == 0

def test_instruction_addi_to_eol():
    asm_state = AsmState()

    state = Instruction()
    asm_line = AsmLine(" \t adDi r0 r1 #-1", 1)
    asm_line.index = 3

    result = state.run(asm_state, asm_line)
    assert type(result) is LineEnd
    assert asm_state.bytecode[-8] == 19
    assert asm_state.bytecode[-7] == 0
    assert asm_state.bytecode[-6] == 1
    assert asm_state.bytecode[-5] == 0
    assert asm_state.bytecode[-4] == 255
    assert asm_state.bytecode[-3] == 255
    assert asm_state.bytecode[-2] == 255
    assert asm_state.bytecode[-1] == 255