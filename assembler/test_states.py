import pytest

from asm_types import AsmState, AsmLine, Stage, AssemblerError
from states import Label, LineStart, Directive, Instruction, LineEnd


def test_start_to_label():
    asm_state = AsmState()
    line = AsmLine("HeLLo", 100)

    state = LineStart()
    result = state.run(asm_state, line)
    assert(result is Label)

def test_start_to_directive():
    asm_state = AsmState()
    line = AsmLine(" \t \t \t.INT", 100)

    state = LineStart()
    result = state.run(asm_state, line)
    assert(result is Directive)

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
    result = state.run(asm_state, line)
    assert(result is Instruction)
    assert(asm_state.stage == Stage.Code)

def test_start_to_comment():
    asm_state = AsmState()
    line = AsmLine(" \t \t \t;This is a comment", 100)

    state = LineStart()
    result = state.run(asm_state, line)
    assert(result is LineEnd)

    line.line = ";This is a comment at the beginning"
    line.index = 0
    result = state.run(asm_state, line)
    assert (result is LineEnd)

def test_start_empty_line():
    asm_state = AsmState()
    line = AsmLine("", 100)

    state = LineStart()
    result = state.run(asm_state, line)
    assert(result is LineEnd)

def test_start_space_tab_line():
    asm_state = AsmState()
    line = AsmLine("\t\t\t\t                                                                    ", 100)

    state = LineStart()
    result = state.run(asm_state, line)
    assert(result is LineEnd)

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

    assert result is Directive
    assert line.index == 12

def test_label_to_instruction():
    asm_state = AsmState()
    line = AsmLine("HELLO \t\t\t    TRP #0", 55)

    state = Label()
    result = state.run(asm_state, line)

    assert result is Instruction
    assert line.index == 13