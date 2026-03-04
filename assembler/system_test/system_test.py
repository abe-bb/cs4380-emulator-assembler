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
    args = ["python", assembler_path, input_dir + input_name + ".asm"]
    return subprocess.run(args)

def run_and_cmp(file_pair_prefix: str):
    result = run_assembler(file_pair_prefix)

    assert result.returncode == 0
    assert cmp_output_expected(file_pair_prefix)

def test_directive_in_code_section():
    result = run_assembler("directive_in_code_section")

    assert result.returncode == 2

def test_no_input_file_provided():
    result = subprocess.run(["python", assembler_path])

    assert result.returncode == 1

def test_missing_input_file():
    result = run_assembler("this_file_doesnt_exist")

    assert result.returncode == 1

def test_given_example():
    run_and_cmp("given_example")
    
def test_simply_exit():
    run_and_cmp("simply_exit")

def test_bunch_of_comments():
    run_and_cmp("bunch_of_comments")

def test_jmp_to_lbl():
    run_and_cmp("jmp_to_lbl")

def test_int_directive():
    run_and_cmp("int_directive")

def test_byt_integer_directive():
    run_and_cmp("byt_integer_directive")

def test_byte_char_directive():
    run_and_cmp("byt_char_directive")

# session fixture that deletes all the assembler binary files after the tests run
@pytest.fixture(scope="session", autouse=True)
def clean_binary_outputs():
    # take no action before running tests
    yield
    # clean up files after test run
    for file_name in listdir(input_dir):
        if file_name.endswith(".bin"):
            os.remove(input_dir + file_name)
