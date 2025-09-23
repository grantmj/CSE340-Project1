# CSE340 Fall 2025 Project 1: A Simple Compiler!*

**Due:** Thursday, September 26 2025 by 11:59 pm MST  
\*© 2018–2025 Rida A. Bazzi

---

## 1 Introduction
I will start with a high-level description of the project and its tasks, and in subsequent sections I will give a detailed description of these tasks. The goal of this project is to implement a simple compiler for a simple programming language. To implement this simple compiler, you will write a recursive-descent parser and use some simple data structures to implement basic semantic checking and execute the input program.

The input to your compiler has four parts:

1. The first part of the input is the **TASKS** section. It contains a list of one or more numbers of tasks to be executed by the compiler.
2. The second part of the input is the **POLY** section. It contains a list of polynomial declarations.
3. The third part of the input is the **EXECUTE** section. It contains a sequence of **INPUT**, **OUTPUT**, and assignment statements.
4. The fourth part of the input is the **INPUTS** section. It contains a sequence of integers that will be used as the input to **INPUT** statements in the **EXECUTE** section.

Your compiler will parse the input and produces a syntax error message if there is a syntax error. If there is no syntax error, your compiler will analyze semantic errors. If there are no syntax and no semantic errors, your compiler will perform other tasks whose numbers are listed in the **TASKS** section. Task 2 requires your compiler to execute the **EXECUTE** section and produces the output that should be produced by the **OUTPUT** statements. Tasks 3, 4, and 5 require your compiler to rewrite the polynomial declarations in the **POLY** section.

**Document roadmap**

- Section 2 describes the input format.  
- Section 3 gives a general overview of the tasks.  
- Section 4 describes the expected output when the syntax or semantics are not correct.  
- Section 5 describes Task 2 and program outputs.  
- Section 6 describes Task 3.  
- Section 7 describes Task 4.  
- Section 8 describes Task 5.  
- Section 9 describes the requirements for your solution.

> **Note:** Nothing in this project is particularly hard, but it is larger than other projects that you have done in the past for other classes. The size of the project can make it feel unwieldy. To deal with the size of the project, it is important to have a good idea of what the requirements are. To do so, you should read this document a couple of times. Then, you should have an implementation plan. I make the task easier by providing an implementation guide that addresses some issues that you might encounter in implementing a solution. Once you have a good understanding and a good plan, you can start coding.

---

## 2 Input Format

### 2.1 Grammar and Tokens
The input of your program is specified by the following context-free grammar:

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

pgsql
Copy code

The code that we provided includes a declaration of a class `LexicalAnalyzer` with methods `GetToken()` and `peek()`. Also, an `expect()` function is provided. Your parser will use the functions provided to `peek()` at tokens or `expect()` tokens as needed. You must not change these provided functions; you just use them as provided. In fact, when you submit the code, you should not submit the files `inputbuf.cc`, `inputbuf.h`, `lexer.cc` or `lexer.h` on gradescope; when you submit the code, the submission site will automatically provide these files, so it is important not to modify these files in your implementation.

To use the provided methods, you should first instantiate a lexer object of the class `LexicalAnalyzer` and call the methods on this instance. You should only instantiate one lexer object. If you try to instantiate more than one, this will result in errors.

The definition of the tokens is given below for completeness (you can ignore it for the most part if you want).

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

pgsql
Copy code

What you need to do is write a parser to parse the input according to the grammar and produce a syntax error message if there is a syntax error. Your program will also check for semantic errors and, depending on the tasks list, will execute more semantic tasks. To achieve that, your parser will store the program in appropriate data structures that facilitate semantic analysis and allow your compiler to execute the statement list in the `execute_section`. For now, do not worry how that is achieved. I will explain that in detail, partly in this document and more fully in the implementation guide document.

### 2.2 Examples

**EXAMPLE 1**
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

makefile
Copy code
Output:
17
3

pgsql
Copy code
The sequence of numbers at the end (in the `input_section`) is ignored because there are no INPUT statements.

**EXAMPLE 2**
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

makefile
Copy code
Output:
2

markdown
Copy code
The first `INPUT` sets `X=1`; the second sets `Y=2`. Remaining inputs are unused.

**EXAMPLE 3**
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

nginx
Copy code
Missing semicolon on line 4 →
SYNTAX ERROR !!!!!&%!!!!&%!!!!!!

markdown
Copy code

**EXAMPLE 4**
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

makefile
Copy code
Output:
2
6

markdown
Copy code

**EXAMPLE 5**
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

makefile
Copy code
Output:
Semantic Error Code IM-4: 5

vbnet
Copy code

---

## 3 Tasks and their priorities

- **Task 1 – Syntax and Semantic error checking**
  1. Syntax checking  
  2. Semantic error checking
- **Task 2 – Output**: produce output of OUTPUT statements when no errors.
- **Task 3 – Sorting and combining monomials in a monomial list**: combine powers and order by header parameter order (e.g., `x2z` → `zx2` if header is `(z, x, y)`).
- **Task 4 – Sorting and combining monomial lists in a term list**: after Task 3 normalization, combine identical monomial lists by summing coefficients; order by first appearance.
- **Task 5 – Polynomial expansion and simplification**: expand all parenthesized lists; then apply Tasks 3 and 4.

**Multiple tasks in TASKS section:** If Task 1 is listed and reports an error, print only its error(s) and stop. If Task 1 is listed and finds no error, it produces no output; then output other listed tasks in numeric order. If Task 1 is not listed, you still perform it silently; on error, print nothing; otherwise print the listed tasks’ outputs in numeric order. Duplicate task numbers are ignored beyond their first occurrence.

Example:
TASKS
1 3 4 1 2 3

vbnet
Copy code
is equivalent to
TASKS
1 2 3 4

yaml
Copy code

---

## 4 Task 1 – Syntax and Semantic Checking

### 4.1 Syntax Checking
If the input is not correct syntactically, your program should output exactly:
SYNTAX ERROR !!!!!&%!!!!&%!!!!!!

bash
Copy code
and exit (no other output).

### 4.2 Semantic Checking
Detect and report exactly one class of semantic error per test case (tests won’t mix types).

- **DMT-12 – Polynomial declared more than once**  
  Output format:
Semantic Error Code DMT-12: <line no 1> <line no 2> ... <line no k>

sql
Copy code
List lines **of duplicate occurrences only**, sorted; repeat a line number for multiple duplicates on the same line.

- **IM-4 – Invalid monomial name**  
Univariate headers without parameter list must use lowercase `x`. For headers with an explicit parameter list, only those variables may appear in the body. Output:
Semantic Error Code IM-4: <line no 1> <line no 2> ... <line no k>

nginx
Copy code
One entry **per invalid occurrence** (repeat line number if multiple occur on same line); sort ascending.

- **AUP-13 – Attempted evaluation of undeclared polynomial**  
Name used in evaluation has no matching declaration. Output:
Semantic Error Code AUP-13: <line no 1> <line no 2> ... <line no k>

typescript
Copy code

- **NA-7 – Wrong number of arguments**  
Mismatch between evaluation arguments and declaration parameters. Output:
Semantic Error Code NA-7: <line no 1> <line no 2> ... <line no k>

yaml
Copy code

---

## 5 Task 2 – Program Output

Define precise execution semantics.

### 5.1 Variables and Locations
Associate each variable name in **EXECUTE** with a unique location in `mem` (an integer array). Initialize all to `0`. Maintain a mapping `location(name) -> int`, assigning new indices incrementally upon first sight.

Example mapping for a sample program:
X -> 0
Z -> 1
Y -> 2
W -> 3

css
Copy code

### 5.2 Statements

**5.2.1 Input statements**  
The i’th `INPUT X;` assigns the i’th number from **INPUTS** to `X`:
mem[location("X")] = ith_input

markdown
Copy code

**5.2.2 Output statements**  
`OUTPUT X;` prints:
cout << mem[location("X")] << endl;

markdown
Copy code

**5.2.3 Assignment statements**  
`X = <poly_evaluation>;` stores value `v`:
mem[location("X")] = v;

markdown
Copy code

**5.2.4 Polynomial Evaluation**  
- **Argument evaluation**:  
  - `ID "X"` → current value of `X` (0 if never assigned).  
  - nested polynomial evaluation → its value.  
  - `NUM` → its integer value.
- **Argument/Parameter correspondence**: i’th argument ↔ i’th parameter (or implicit single `x` if header has no list).
- **Coefficient** → integer value.  
- **Exponent** → integer value.  
- **Monomial**:  
  - `X` → value of argument bound to `X`.  
  - `X^e` → `v^e` (with `v` the bound argument value).  
- **Monomial list**: product of monomials.  
- **Term**:  
  - `c * monomial_list` → `c * v`.  
  - `monomial_list` → `1 * v`.  
  - `parenthesized_list` → its value.  
- **Parenthesized list** `(TL1)(TL2)...(TLk)` → product of term-list values.  
- **Term list** `t1 op1 t2 ...` → apply +/− sequentially.  
- **Polynomial body** → term-list value.  
- **Polynomial** → polynomial-body value.

### 5.3 Assumptions
1. If there is a polynomial declaration with a given name, there is no variable with the same name.  
2. If `mem` is an array, size 1000 is sufficient for tests; handle overflow gracefully.

---

## 6 Task 3 – Sorting and combining monomials in a monomial list

Sort monomials in each monomial list according to the parameter order of the header. Combine repeated variables by summing exponents.

**Example**
TASKS
2 1 3
POLY
F1 = x^2 + 1;
F4(x,z,y) = y x^2 x^3 + y x z + z^2 y + 1;
EXECUTE
INPUT X;
INPUT Y;
W = F4(X,Y, F2(X,Y,X));
OUTPUT Y;
INPUTS
5 2 3 18 19 22 33 12 11 16

makefile
Copy code
Output:
F1 = x^2 + 1 ;
F2(x,z,y) = x^5 y + x z y + z^2 y + 1 ;

sql
Copy code

**POWER-array method**  
Represent a monomial list as an array `POWER[1..n]` aligned to parameters `[v1,...,vn]`. Initialize zeros; for each monomial `v` add 1; for `v^p` add `p`. Then print variables in parameter order with nonzero powers (omit exponent for power 1).

**Task 3 Output format:**
POLY - SORTED MONOMIAL LISTS
<poly declaration 1 with combined monomial lists>
...
<poly declaration k with combined monomial lists>

markdown
Copy code

**Algorithm 1: Ordering and combining monomials**
Input: Monomial List ML = m1 m2 ... mk
Input: Param List PL = v1, v2, ..., vn
Output: Monomial List ML' = m1' m2' ... mk'

Initialization:
for i ← 1 to n:
POWER[i] = 0
for j ← 1 to k:
if mj = vi for some vi:
POWER[i] += 1
else if mj = vi^p for some vi and p:
POWER[i] += p

Building TL':
TL' = ""
for i ← 1 to n:
if POWER[i] = 1:
append vi to TL'
if POWER[i] > 1:
append vi^POWER[i] to TL'

pgsql
Copy code
Each polynomial’s monomial lists are processed independently.

---

## 7 Task 4 – Sorting and combining monomial lists in a term list

First perform Task 3 on each monomial list. Then, within a term list, combine **identical monomial lists** by summing coefficients, respecting operators; drop zero-coefficient terms. Parenthesized lists are processed recursively. Final order is by **first appearance** of each distinct monomial list.

**Task 4 Output format:**
POLY - COMBINED MONOMIAL LISTS
<poly declaration 1 with combined monomial lists>
...
<poly declaration k with combined monomial lists>

yaml
Copy code

**Algorithm 2: Combining monomial lists (high level)**

Data structure for terms:
struct TermNode {
Kind kind; // MONOMLIST or PARENLIST
Operator op; // PLUS or MINUS
int coefficient; // for MONOMLIST
vector<int> monomial_list; // for MONOMLIST
vector<vector<TermNode>> parenthesized_list; // for PARENLIST
}

makefile
Copy code

Processing:
Input: Term List TL = t1 t2 ... tk
Output: Term List TL' = t1' t2' ... tk'

for i ← 1..k:
if ti.kind = MONOMLIST:
tr = ti; mark ti processed
for j ← i+1..k:
if tj.kind = MONOMLIST
and tj.monomial_list == ti.monomial_list
and tj not processed:
c = (tr.op as ±1)*tr.coefficient + (tj.op as ±1)*tj.coefficient
if c > 0: tr.op = PLUS, tr.coefficient = c
else : tr.op = MINUS, tr.coefficient = -c
mark tj processed
if tr.coefficient != 0: append tr to TL'
else:
recursively process ti.parenthesized_list and append

yaml
Copy code

---

## 8 Task 5 – Polynomial expansion and simplification

Expand by multiplying out **all** parenthesized lists to remove them entirely, then apply Task 3 and Task 4. Order the final terms by:

1. **Degree descending** (sum of exponents).
2. **Tie-breaker:** reverse lexicographic order of the monomial vector (aligned to header parameter order).

Example with parameters `(x, y, z)`:
P(x, y, z) = (x + y)(x + z)(y + z)
→ P(x, y, z) = x^2y + x^2z + xy^2 + 2xyz + xz^2 + y^2z + yz^2

less
Copy code
Monomial vectors (degree 3): `[2,1,0] > [2,0,1] > [1,2,0] > [1,1,1] > [1,0,2] > [0,2,1] > [0,1,2]`.

**Task 5 Output format:**
POLY - EXPANDED
<poly declaration 1 expanded>
...
<poly declaration k expanded>

less
Copy code

**Algorithm 3: Printing a term list**
Input: Term List TL = t1 t2 ... tk

for i ← 1..k:
if i != 1: print ti.operator
else: print operator only if MINUS

if ti.kind = MONOMLIST:
if ti.coefficient = 1:
if some variable has non-zero power:
print variables with non-zero powers (in parameter order, omitting ^1)
else:
print 1
else if ti.coefficient != 0:
print ti.coefficient
print variables with non-zero powers
// coefficient 0 → print nothing
else:
print '('
recursively print parenthesized_list
print ')'

yaml
Copy code
Note: Some tests have no parenthesized lists—these focus on ordering.

---

## 9 Requirements
Write a program that generates the correct output as described. Start by writing the parser and make sure it correctly parses the input before implementing the rest. You will be provided example test cases; they are **not** comprehensive. Develop your own tests.

---

## 10 Instructions

- Read this document carefully.
- When the implementation guide is posted, read it carefully.
- Download `lexer.cc`, `lexer.h`, `inputbuf.cc`, and `inputbuf.h` and learn the provided functions.
- Design your solution before coding (especially data structures). You can and should write the parser first.
- Compile with **GCC (g++ 11.2.0)** on **Ubuntu 22.04**. You may develop elsewhere but must compile/test on Ubuntu/GCC.
- Test with the provided cases using `test1.sh`; then write your own.
- Submit via gradescope. You can activate any prior submission’s grade (even during late period).

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
g++ test_program.cpp
g++ test_program.cpp -o hello.out
g++ -std=c++11 test_program.cpp -o hello.out

markdown
Copy code
Useful flags:
- `-o path` (output name)
- `-g` / `-ggdb` (debug info)
- `-Wall` (warnings)
- `-std=c++11` (use C++11)

Multiple files:
g++ file1.cpp file2.cpp file3.cpp

or
g++ -c file1.cpp
g++ -c file2.cpp
g++ -c file3.cpp
g++ file1.o file2.o file3.o

lua
Copy code

### 12.2 Testing your code on Ubuntu
Programs should **not** open files directly; use **stdin/stdout** only. The provided lexer reads from stdin.

Run:
./a.out
./a.out < input_data.txt
./a.out > output_file.txt
./a.out < input_data.txt > output_file.txt

markdown
Copy code

**Test cases**: For each test have `test_name.txt` and `test_name.txt.expected`. Run and diff:
./a.out < test_name.txt > program_output.txt
diff -Bw program_output.txt test_name.txt.expected

markdown
Copy code

Use the provided `test1.sh`:
- Put `tests.zip` and `test1.sh` in your project dir.
- `unzip tests.zip` → creates `tests/`
- `chmod +x test1.sh`
- Compile (`a.out` expected name)
- `./test1.sh`