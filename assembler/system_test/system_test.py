import filecmp
import os
import subprocess
from subprocess import CompletedProcess

from os import listdir
import pytest

# These are system tests. Each test follows a similar set of instructions.
# 1. The assembler is called with a .asm file
# 2. The return code of the assembler validated
# 3. The output binary file is checked against a handcrafted binary file to check assembler behavior
#
# The input directory contains the .asm files and is also where the assembler will place the
# output binaries as per the project 2 spec.
#
# The expected directory contains the handcrafted .bin files that will be compared against the assembler outputs

# path to folder with expected output files
expected_dir = "expected/"
# path to folder with input files
input_dir = "input/"
# path to assembler
assembler_path = "../asm4380.py"

def cmp_output_expected(input_name: str) -> bool:
    return filecmp.cmp(expected_dir + input_name + ".bin", input_dir + input_name + ".bin", shallow=False)

def run_assembler(input_name: str) -> CompletedProcess:
    args = ["python", "../asm4380.py", input_dir + input_name + ".asm"]
    return subprocess.run(args)

def test_directive_in_code_section():
    result = run_assembler("directive_in_code_section")

    assert result.returncode == 2

def test_missing_input_file():
    result = run_assembler("this_file_doesnt_exist")

    assert result.returncode == 1

def test_given_example():
    test_files = "given_example"
    result = run_assembler(test_files)

    assert result.returncode == 0
    assert cmp_output_expected(test_files)
    
def test_simply_exit():
    test_files = "simply_exit"
    result = run_assembler(test_files)

    assert result.returncode == 0
    assert cmp_output_expected(test_files)

def test_bunch_of_comments():
    test_files = "bunch_of_comments"
    result = run_assembler(test_files)

    assert result.returncode == 0
    assert cmp_output_expected(test_files)

def test_jmp_to_lbl():
    test_files = "jmp_to_lbl"
    result = run_assembler(test_files)

    assert result.returncode == 0
    assert cmp_output_expected(test_files)

# session fixture that deletes all the assembler binary files after the tests run
@pytest.fixture(scope="session", autouse=True)
def clean_binary_outputs():
    # take no action before running tests
    yield
    # clean up files after test run
    for file_name in listdir(input_dir):
        if file_name.endswith(".bin"):
            os.remove(input_dir + file_name)