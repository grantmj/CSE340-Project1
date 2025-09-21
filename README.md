# CSE340 Fall 2025 Project 1: A Simple Compiler!*

**Due:** Thursday, September 26 2025 by 11:59 pm MST

\*© 2018–2025 Rida A. Bazzi

---

## 1 Introduction
I will start with a high-level description of the project and its tasks, and in subsequent sections I will give a detailed description of these tasks. The goal of this project is to implement a simple compiler for a simple programming language. To implement this simple compiler, you will write a recursive-descent parser and use some simple data structures to implement basic semantic checking and execute the input program.

The input to your compiler has four parts:

1. The first part of the input is the TASKS section. It contains a list of one or more numbers of tasks to be executed by the compiler.
2. The second part of the input is the POLY section. It contains a list of polynomial declarations.
3. The third part of the input is the EXECUTE section. It contains a sequence of INPUT, OUTPUT, and assignment statements.
4. The fourth part of the input is the INPUTS section. It contains a sequence of integers that will be used as the input to INPUT statements in the EXECUTE section.

Your compiler will parse the input and produces a syntax error message if there is a syntax error. If there is no syntax error, your compiler will analyze semantic errors. If there are no syntax and no semantic errors, your compiler will perform other tasks whose numbers are listed in the TASKS section. Task 2 requires your compiler to execute the EXECUTE section and produces the output that should be produced by the OUTPUT statements. Tasks 3, 4, and 5 require your compiler to rewrite the polynomial declarations in the POLY section.

The remainder of this document is organized as follows.

- The second section describes the input format.
- The third section gives a general overview of the tasks.
- The fourth section describes the expected output when the syntax or semantics are not correct.
- The fourth section describes the output when the program syntax and semantics are correct.
- The fifth section describes Task 2 and program outputs.
- The sixth section describes Task 3.
- The seventh section describes Task 4.
- The eighth section describes Task 5.
- The ninth section describes the requirements for your solution.

**Note:** Nothing in this project is particularly hard, but it is larger than other projects that you have done in the past for other classes. The size of the project can make it feel unwieldy. To deal with the size of the project, it is important to have a good idea of what the requirements are. To do so, you should read this document a couple of times. Then, you should have an implementation plan. I make the task easier by providing an implementation guide that addresses some issues that you might encounter in implementing a solution. Once you have a good understanding and a good plan, you can start coding.

---

## 2 Input Format

### 2.1 Grammar and Tokens
The input of your program is specified by the following context-free grammar:

```
program → tasks_section poly_section execute_section inputs_section
tasks_section → TASKS num_list
num_list → NUM
num_list → NUM num_list
poly_section → POLY poly_decl_list
poly_decl_list → poly_decl
poly_decl_list → poly_decl poly_decl_list
poly_decl → poly_header EQUAL poly_body SEMICOLON
poly_header → poly_name
poly_header → poly_name LPAREN id_list RPAREN
id_list → ID
id_list → ID COMMA id_list
poly_name → ID
poly_body → term_list
term_list → term
term_list → term add_operator term_list
term → monomial_list
term → coefficient monomial_list
term → coefficient
term → parenthesized_list
monomial_list → monomial
monomial_list → monomial monomial_list
monomial → ID
monomial → ID exponent
exponent → POWER NUM
parenthesized_list → LPAREN term_list RPAREN
parenthesized_list → LPAREN term_list RPAREN parenthesized_list
add_operator → PLUS
add_operator → MINUS
coefficient → NUM
execute_section → EXECUTE statement_list
statement_list → statement
statement_list → statement statement_list
statement → input_statement
statement → output_statement
statement → assign_statement
input_statement → INPUT ID SEMICOLON
output_statement → OUTPUT ID SEMICOLON
assign_statement → ID EQUAL poly_evaluation SEMICOLON
poly_evaluation → poly_name LPAREN argument_list RPAREN
argument_list → argument
argument_list → argument COMMA argument_list
argument → ID
argument → NUM
argument → poly_evaluation
inputs_section → INPUTS num_list
```

The code that we provided includes a declaration of a class `LexicalAnalyzer` with methods `GetToken()` and `peek()`. Also, an `expect()` function is provided. Your parser will use the functions provided to `peek()` at tokens or `expect()` tokens as needed. You must not change these provided functions; you just use them as provided. In fact, when you submit the code, you should not submit the files `inputbuf.cc`, `inputbuf.h`, `lexer.cc` or `lexer.h` on gradescope; when you submit the code, the submission site will automatically provide these files, so it is important not to modify these files in your implementation.

To use the provided methods, you should first instantiate a lexer object of the class `LexicalAnalyzer` and call the methods on this instance. You should only instantiate one lexer object. If you try to instantiate more than one, this will result in errors.

The definition of the tokens is given below for completeness (you can ignore it for the most part if you want).

```
char = a | b | ... | z | A | B | ... | Z | 0 | 1 | ... | 9
letter = a | b | ... | z | A | B | ... | Z
pdigit = 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9
digit = 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9

SEMICOLON = ;
COMMA = ,
PLUS = +
MINUS = -
POWER = ^
EQUAL = =
LPAREN = (
RPAREN = )
TASKS = (T).(A).(S).(K).(S)
POLY = (P).(O).(L).(Y)
EXECUTE = (E).(X).(E).(C).(U).(T).(E)
INPUT = (I).(N).(P).(U).(T)
OUTPUT = (O).(U).(T).(P).(U).(T)
INPUTS = (I).(N).(P).(U).(T).(S)
NUM = 0 | pdigit . digit*
ID = letter . char*
```

What you need to do is write a parser to parse the input according to the grammar and produce a syntax error message if there is a syntax error. Your program will also check for semantic errors and, depending on the tasks list, will execute more semantic tasks. To achieve that, your parser will store the program in appropriate data structures that facilitate semantic analysis and allow your compiler to execute the statement list in the `execute_section`. For now, do not worry how that is achieved. I will explain that in detail, partly in this document and more fully in the implementation guide document.

### 2.2 Examples
The following are five examples of input (to your compiler) with corresponding outputs. The output will be explained in more detail in later sections. Each of these examples has task numbers 1 and 2 listed in the `tasks_section`. They have the following meanings:

- The number 1 listed means that your program should perform syntax and semantic checking.
- The number 2 listed means that your program should produce the output of the output statements if there are no syntax and no semantic errors.

**EXAMPLE 1**
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
This example shows two polynomial declarations and a EXECUTE section in which the polynomials are evaluated with arguments 4 and 2 respectively. The output of the program will be
```
17
3
```
The sequence of numbers at the end (in the `input_section`) is ignored because there are no INPUT statements.

**EXAMPLE 2**
```
TASKS
1 2

POLY
F = x^2 + 1;
G = x + 1;

EXECUTE
INPUT X;
INPUT Y;
X = F(X);
Y = G(Y);
OUTPUT X;

INPUTS
1 2 3 18 19
```
This is similar to the previous example, but here we have two INPUT statements. The first INPUT statement reads a value for X from the sequence of numbers and X gets the value 1. The second INPUT statement reads a value for Y which gets the value 2. Here the output will be
```
2
```
Note that the values 3, 18 and 19 are not read and do not affect the execution of the program.

**EXAMPLE 3**
```
1: TASKS
2: 1 2
3: POLY
4: F = x^2 + 1
5: G = x + 1;
6: EXECUTE
7: INPUT X;
8: INPUT Y;
9: X = F(X);
10: Y = G(Y);
11: OUTPUT X;
12: INPUTS
13: 1 2 3 18 19
```
In this example I added line numbers to be able to refer to specific lines of the input program. Line numbers are not part of the input to your compiler and are only shown for references. In this example, which looks almost the same as the previous example, there is a syntax error because there is a missing semicolon on line 4. The output of the program should be
```
SYNTAX ERROR !!!!!&%!!!!&%!!!!!!
```

**EXAMPLE 4**
```
1: TASKS
2: 1 2
3: POLY
4: F = x^2 + 1;
5: G(X,Y) = X Y^2 + X Y;
6: EXECUTE
7: INPUT Z;
8: INPUT W;
9: X = F(Z);
10: Y = G(Z,W);
11: OUTPUT X;
12: OUTPUT Y;
13: INPUTS
13: 1 2 3 18 19
```
In this example, the polynomial G has two variables which are given explicitly (in the absence of explicitly named variables, the variable is lower case x by default). The output is
```
2
6
```

**EXAMPLE 5**
```
1: TASKS
2: 1 2
3: POLY
4: F = x^2 + 1;
5: G(X,Y) = X Y^2 + X Z;
6: EXECUTE
7: INPUT Z;
8: INPUT W;
9: X = F(Z);
10: Y = G(Z,W);
11: OUTPUT X;
12: OUTPUT Y;
12: INPUTS
13: 1 2 3 18 19
```
This example is similar to the previous one but it has a problem. The polynomial G is declared with two variables X and Y but its equation (called `poly_body` in the grammar) has Z which is different from X and Y. The output captures this error (see below for error codes and their format)
```
Semantic Error Code IM-4: 5
```

---

## 3 Tasks and their priorities
The task numbers specify what your program should do with the input program. The tasks have the following functionalities:

- **Task 1 – Syntax and Semantic error checking**  
  Task 1 is one of the larger tasks and, but it is not graded as one big task (grading is described later in the document). Task 1 has the following functionalities:
  1. Syntax checking
  2. Semantic error checking

- **Task 2 – Output:**  
  Task 2 requires your compiler to produce the output that should be produced by the output statements of the program.

- **Task 3 – Sorting and combining monomials in a monomial list:**  
  Task 3 requires your compiler to rewrite the polynomial declarations so that different monomials for the same variable are combined (`x2x3 = x5` for example) and sorted in the order in which they appear in the polynomial list of parameters. For example, if the polynomial declaration is `P (z, x, y) = ...` has z listed as the first parameter and the body of the polynomial contains the monomial list `x2z`, the monomial list will be rewritten `zx2`.

- **Task 4 – Sorting and combining monomial lists in a term list:**  
  For this task, you are asked to combine different identical monomial lists in a term list. For example, if your polynomial declaration is `P (x, y, z) = xy2 − 2xyz + y2x + xyz`, then, after the combining, you get `P (x, y, z) = 2xy2 − xyz`. This tasks requires that you execute Task 3 as a first step.

- **Task 5 – Polynomial expansion and simplification:**  
  For this task, you are asked to expand the polynomial by multiplying all the parenthesized lists. For example, if the polynomial is `P (x) = (1 + x)(x + 1 + (1 + x)(1 + x))`, the result of the expansion will be `P (x) = 2 + 5x + 4x2 + x3`. This tasks requires the functionality of Tasks 3 and 4.

Detailed descriptions of these tasks and what the output should be for each of them is given in the sections that follow. The remainder of this section explains what the output of your program should be when multiple task numbers are listed in the `tasks_section`.

If task 1 is listed in the `tasks_section`, then task 1 should be executed. Remember that task 1 performs syntax error checking and semantic error checking. If the execution of task 1 results in an error, and task 1 is listed in the `tasks_section`, then your program should only output the error messages (as described below) and exits. If task 1 results in an error (syntax or semantic) no other tasks will be executed even if they are listed in the `tasks_section`. If task 1 is listed in the `tasks_section` and does not result in an error message, then task 1 produces no output. In that case, the outputs of the other tasks that are listed in `tasks_section` should be produced by the program. The order of these outputs should be according to the task numbers. So, first the output of task 2 is produced (if task 2 is listed in `tasks_section`), then the output of task 3 is produced (if task 3 is listed in `tasks_section`) and so on.

If task 1 is not listed in the `tasks_section`, task 1 still needs to be executed. If task 1’s execution results in an error, then your program should output nothing in this case. If task 1 is not listed and task 1’s execution does not result in an error, then the outputs of the other tasks that are listed in `tasks_section` should be produced by the program. The order of these outputs should be according to the task numbers. So, first the output of task 2 is produced, then the output of task 3 is produced (if task 3 is listed in `tasks_section`) and so on.

You should keep in mind that tasks are not necessarily listed in order in the `tasks_section` and they can even be repeated. For instance, we can have the following TASKS section:
```
TASKS
1 3 4 1 2 3
```
In this example, some tasks are listed more than once. Later occurrences are ignored. So, the `tasks_section` above is equivalent to
```
TASKS
1 2 3 4
```
In the implementation guide, I explain a simple way to read the list and sort the task numbers using a boolean array.

---

## 4 Task 1 – Syntax and Semantic Checking

### 4.1 Syntax Checking
If the input is not correct syntactically, your program should output
```
SYNTAX ERROR !!!!!&%!!!!&%!!!!!!
```
If there is syntax error, the output of your program should exactly match the output given above. No other output should be produced in this case, and your program should exit after producing the syntax error message. The provided `parser.*` skeleton files already have a function that produces the message above and exits the program.

### 4.2 Semantic Checking
Semantic checking also checks for invalid input. Unlike syntax checking, semantic checking requires knowledge of the specific lexemes and does not simply look at the input as a sequence of tokens (token types). I start by explaining the rules for semantic checking. I also provide some examples to illustrate these rules.

- **Polynomial declared more than once – Semantic Error Code DMT-12.**  
  If the same `polynomial_name` is used in two or more different `poly_header`’s, then we have the error polynomial declared more than once. The output in this case should be of the form
  ```
  Semantic Error Code DMT-12: <line no 1> <line no 2> ... <line no k>
  ```
  where `<line no 1>` through `<line no k>` are the numbers of each of the lines in which a duplicate `polynomial_name` appears in a polynomial header. The numbers should be sorted from smallest to largest. For example:
  ```
  1: TASKS
  2: 1 3 4
  3: POLY
  4: F1 =
  5: x^2 + 1;
  6: F2 = x^2 + 1;
  7: F1 = x^2 + 1; F1 = x^2 + 1;
  8: F3 = x^2 + 1;
  9: G = x^2 + 1;
  10: F1 = x^2 + 1;
  11: G(X,Y) = X Y^2 + X Y;
  12: EXECUTE
  13: INPUT Z;
  14: INPUT W;
  15: X = F1(Z);
  16: Y = G(W);
  17: OUTPUT X;
  18: OUTPUT Y;
  19: INPUTS
  20: 1 2 3 18 19
  ```
  Output:
  ```
  Semantic Error Code DMT-12: 7 7 10 11
  ```
  Note that only the line numbers for the duplicates are listed. The line number for the first occurrence of a name is not listed. `7` is printed twice because there are two duplicate declarations for `F1` on line 7.

- **Invalid monomial name – Semantic Error Code IM-4.**  
  There are two kinds of polynomials headers. In the first kind, only the polynomial name (ID) is given and no parameter list (`id_list` in the header) is given. In a polynomial with the first kind of header, the polynomial should be univariate (one variable) and the variable name should be lower case `x`. In polynomials with the second kind of header, the header has the form `polynomial_name LPAREN id_list RPAREN`. In a polynomial evaluation, an ID that appears in the body of a polynomial (in primary) should be equal to one of the variables of the polynomial. Otherwise, we have an invalid monomial name error:
  ```
  Semantic Error Code IM-4: <line no 1> <line no 2> ... <line no k>
  ```
  Print the line number once per invalid occurrence. Sort ascending.

- **Attempted evaluation of undeclared polynomial – Semantic Error Code AUP-13.**  
  If there is no polynomial declaration with the used name:
  ```
  Semantic Error Code AUP-13: <line no 1> <line no 2> ... <line no k>
  ```
  Example:
  ```
  1: TASKS
  2: 1 3 4
  3: POLY
  4: F1 = x^2 + 1;
  5: F2 = x^2 + 1;
  6: F3 = x^2 + 1;
  7: F4 = x^2 + 1;
  8: G1 = x^2 + 1;
  9: F5 = x^2 + 1;
  10: G2(X,Y) = X Y^2 + X Y;
  11: EXECUTE
  12: INPUT Z;
  13: INPUT W;
  14: X = G(Z); X = G(Z);
  15: Y = G2(Z,W);
  16: X = F(Z);
  17: Y = G2(Z,W);
  18: INPUTS
  19: 1 2 3 18 19
  ```
  Output:
  ```
  Semantic Error Code AUP-13: 14 14 16
  ```

- **Wrong number of arguments – Semantic Error Code NA-7.**  
  If the number of arguments in a polynomial evaluation differs from the number of parameters in its declaration:
  ```
  Semantic Error Code NA-7: <line no 1> <line no 2> ... <line no k>
  ```
  Example:
  ```
  1: TASKS
  2: 1 3 4
  3: POLY
  4: F1 = x^2 + 1;
  5: F2 = x^2 + 1;
  6: F3 = x^2 + 1;
  7: F4 = x^2 + 1;
  8: G1 = x^2 + 1;
  9: F5 = x^2 + 1;
  10: G2(X,Y) = X Y^2 + X Y;
  11: EXECUTE
  12: INPUT Z;
  13: INPUT W;
  14: X = G2(X,Y, Z); X = G2(X,Y, Z);
  15: Y = G2(Z,W);
  16: X = F1(Z);
  17: Y = F5(Z,Z);
  18: Y = F5(Z,Z,W);
  19: INPUTS
  20: 1 2 3 18 19
  ```
  Output:
  ```
  Semantic Error Code NA-7: 14 14 17 18
  ```

**Assumption:** Each test case will contain only one kind of semantic error.

---

## 5 Task 2 – Program Output
For task 2, your program should output the results of all the polynomial evaluations in the program. This section defines the meaning of inputs and outputs precisely.

### 5.1 Variables and Locations
Associate each variable name in `EXECUTE` with a unique location in `mem` (an integer array). All variables initialize to `0`. Maintain a `location(name) -> int` mapping; assign new indices incrementally when a name is first seen (e.g., during `INPUT`).

Example program yields locations:
```
X -> 0
Z -> 1
Y -> 2
W -> 3
```

### 5.2 Statements

#### 5.2.1 Input statements
The i’th `INPUT X;` reads the i’th number from `INPUTS`:
```
mem[location("X")] = ith_input
```

#### 5.2.2 Output statements
`OUTPUT X;` prints:
```
cout << mem[location("X")] << endl;
```

#### 5.2.3 Assignment statements
`X = <poly_evaluation>;` stores the evaluated value `v`:
```
mem[location("X")] = v;
```

#### 5.2.4 Polynomial Evaluation
- **Argument evaluation**
  - `ID "X"` → current value of `X` (0 if never assigned).
  - nested polynomial eval → its value.
  - `NUM` → its integer value.
- **Argument/Parameter correspondence**: i’th argument ↔ i’th parameter (or implicit single `x` if header has no list).
- **Coefficient**: numeric literal value.
- **Exponent**: numeric literal value.
- **Monomial**
  - `X` → value of argument bound to `X`.
  - `X^e` → `v^e`.
- **Monomial list**: product of constituent monomials.
- **Term**
  - `c * monomial_list` → `c * v`.
  - `monomial_list` → `1 * v`.
  - `parenthesized_list` → its value.
- **Parenthesized list** `(TL1)(TL2)...(TLk)` → product of each term list’s value.
- **Term list** `t1 op1 t2 op2 ... tk+1` → apply +/− sequentially.
- **Polynomial body** → term list value.
- **Polynomial** → polynomial body value.

### 5.3 Assumptions
1. No variable shares a name with a declared polynomial.
2. If you use a fixed `mem[1000]`, handle overflow gracefully.

---

## 6 Task 3 – Sorting and combining monomials in a monomial list
Sort monomials within each monomial list according to the parameter order in the header. Combine repeated variables by summing exponents.

Example input:
```
TASKS
1 3
POLY
F1 = x^2 + 1;
F2(x,z,y) = y x^2 x^3 + y x z + z^2 y + 1;
EXECUTE
INPUT X;
INPUT Y;
W = F4(X,Y, F2(X,Y,X));
OUTPUT Y;
INPUTS
5 2 3 18 19 22 33 12 11 16
```
Output:
```
F1 = x^2 + 1
F2(x,z,y) = x^5 y + x z y + z^2 y + 1
```

**POWER-array method:** Represent a monomial list as an array of powers aligned to parameters `[v1,...,vn]`. Initialize zeros; for each monomial `v` add 1; for `v^p` add `p`. Then print variables in parameter order with nonzero powers (omit exponent for power 1).

**Task Output format:**
```
POLY - SORTED MONOMIAL LISTS
<poly decl 1 with combined monomial lists>
...
<poly decl k with combined monomial lists>
```

**Algorithm 1** (high level) provided for building combined monomial lists (POWER array accumulation and printing rules).

---

## 7 Task 4 – Sorting and combining monomial lists in a term list
After Task 3 normalization of each monomial list, identify identical monomial lists within a term list and combine their coefficients (respecting +/−). Preserve order by first appearance of each distinct monomial list. Parenthesized lists are processed recursively.

**Printing rules (Algorithm 3 excerpt):**
- Omit printing coefficient `1` unless the term is a pure constant (`all powers zero`).
- Do not print `+` for the very first positive term; do print `-` if the first term is negative.

**Task Output format:**
```
POLY - COMBINED MONOMIAL LISTS
<poly decl 1 with combined monomial lists>
...
<poly decl k with combined monomial lists>
```

---

## 8 Task 5 – Polynomial expansion and simplification
Expand by multiplying out all parenthesized lists (recursively), then apply Task 3 and Task 4. Finally, sort terms by:

1. **Degree descending** (sum of exponents in the monomial list).  
2. **Tie-breaker:** reverse lexicographic order of the monomial vector aligned to header parameter order.

**Example:** For `P(x,y,z) = (x + y)(x + z)(y + z)` the expanded result is:
```
P(x, y, z) = x^2y + x^2z + xy^2 + 2xyz + xz^2 + y^2z + yz^2
```

**Task Output format:**
```
POLY - EXPANDED
<poly decl 1 expanded>
...
<poly decl k expanded>
```

**Note:** Some test cases have no parenthesized lists (partial credit focuses on ordering).

---

## 9 Requirements
Write a program that generates the correct output as described. Start by writing the parser and ensure it passes parsing tests before implementing the rest.

You’ll get example tests; they are **not** comprehensive. Develop additional tests yourself. Submit via gradescope.

---

## 10 Instructions
- Read this document carefully.
- When the implementation guide is posted, read it carefully.
- Download `lexer.cc`, `lexer.h`, `inputbuf.cc`, and `inputbuf.h` and learn the provided functions.
- Design your solution before coding (especially data structures). You can start with the parser.
- Compile with **GCC (g++ 11.2.0)** on **Ubuntu 22.04**. You may develop elsewhere but must compile/test on Ubuntu/GCC.
- Use `test1.sh` with provided tests; also create your own tests.
- Submit on gradescope. You can activate any prior submission’s grade (even during late period).

**Policies**
- Use **C++11** only.
- Assignments are **individual**.
- Submit code **via gradescope** only.
- Get familiar with Ubuntu and GCC.

---

## 11 Evaluation
Autograded on gradescope. Parsing has **no partial credit**—you must pass all parsing tests. Other categories are proportional to passed tests.

Breakdown:
1. Task 1 – Parsing: **30%**
2. Task 1 – Semantic Error Code 1: **7.5%**
3. Task 1 – Semantic Error Code 2: **7.5%**
4. Task 1 – Semantic Error Code 3: **7.5%**
5. Task 1 – Semantic Error Code 4: **7.5%**
6. Task 2 – Program Output: **15%**
7. Task 3 – Monomial list combine/sort: **10%**
8. Task 4 – Term list combine: **15%**
9. Task 5 – Expansion & simplification: **15%**

Total 115% (15% bonus).

---

## 12 General instructions for all programming assignments

### 12.1 Compiling your code with GCC
Use `g++` for C++:
```
g++ test_program.cpp
g++ test_program.cpp -o hello.out
g++ -std=c++11 test_program.cpp -o hello.out
```
Useful flags:
- `-o path` (output name)
- `-g` / `-ggdb` (debug info)
- `-Wall` (warnings)
- `-std=c++11` (use C++11)

Multiple files:
```
g++ file1.cpp file2.cpp file3.cpp
# or
g++ -c file1.cpp
g++ -c file2.cpp
g++ -c file3.cpp
g++ file1.o file2.o file3.o
```

### 12.2 Testing your code on Ubuntu
Programs should **not** open files directly; use **stdin/stdout** only. The provided lexer reads from stdin.

Run:
```
./a.out
./a.out < input_data.txt
./a.out > output_file.txt
./a.out < input_data.txt > output_file.txt
```

**Test cases**: For each test have `test_name.txt` and `test_name.txt.expected`. Run and diff:
```
./a.out < test_name.txt > program_output.txt
diff -Bw program_output.txt test_name.txt.expected
```

Use the provided `test1.sh`:
- Put `tests.zip` and `test1.sh` in your project dir.
- `unzip tests.zip` → creates `tests/`
- `chmod +x test1.sh`
- Compile (`a.out` expected name)
- `./test1.sh`

---

### Quick TOC
- Introduction
- Input Format
  - Grammar and Tokens
  - Examples
- Tasks and their priorities
- Task 1 – Syntax and Semantic Checking
  - Syntax Checking
  - Semantic Checking
- Task 2 – Program Output
  - Variables and Locations
  - Statements (Input / Output / Assignment / Polynomial Evaluation)
  - Assumptions
- Task 3 – Sorting and combining monomials in a monomial list
- Task 4 – Sorting and combining monomial lists in a term list
- Task 5 – Polynomial expansion and simplification
- Requirements
- Instructions
- Evaluation
- General instructions for all programming assignments
  - Compiling your code with GCC
  - Testing your code on Ubuntu
