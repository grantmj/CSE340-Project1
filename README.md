# CSE340-Project1: Simple Compiler

A recursive descent parser and compiler for a simple polynomial programming language.

## Project Overview

This project implements a compiler with the following capabilities:
- **Task 1**: Syntax and semantic error checking ✅
- **Task 2**: Program execution (future implementation)
- **Task 3**: Polynomial monomial sorting (future implementation) 
- **Task 4**: Term list simplification (future implementation)
- **Task 5**: Polynomial expansion (future implementation)

## Current Implementation Status

### ✅ Task 1 - Complete and Verified
- **Syntax Checking**: Detects and reports syntax errors
- **Semantic Error Checking**: Implements all 4 error types:
  - DMT-12: Duplicate polynomial declarations
  - IM-4: Invalid monomial names
  - AUP-13: Attempted use of undeclared polynomials
  - NA-7: Wrong number of arguments in polynomial evaluation

### Test Results
- **All 22 provided tests pass** (100% success rate)
- Comprehensive error detection and reporting
- Proper task flow logic implementation

## Building and Running

```bash
# Compile the parser
g++ -std=c++11 -o parser *.cc

# Run with input file
./parser < input.txt

# Run tests (if test script available)
./run_tests.sh
```

## Input Format

The compiler expects input with four sections:
1. **TASKS**: List of task numbers to execute
2. **POLY**: Polynomial declarations
3. **EXECUTE**: Program statements (INPUT, OUTPUT, assignments)
4. **INPUTS**: Integer values for INPUT statements

## Example

```
TASKS
1 2

POLY
F = x^2 + 1;
G = x + 1;

EXECUTE
X = F(4);
Y = G(2);
OUTPUT X;
OUTPUT Y;

INPUTS
1 2 3 18 19
```

## Implementation Details

- **Recursive Descent Parser**: Complete grammar implementation
- **Error Handling**: Robust syntax and semantic error detection
- **Data Structures**: Efficient storage for polynomial declarations and evaluations
- **Memory Management**: Clean resource handling and proper exit codes

## Course Information

- **Course**: CSE 340 - Principles of Programming Languages
- **Semester**: Fall 2025
- **Due Date**: September 26, 2025
