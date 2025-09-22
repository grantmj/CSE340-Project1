# CSE 340 Project 1 — Implementation Guide (README)

CSE 340 — Fall 2025  
Author: **Rida Bazzi**

> This is a faithful, plain-text/Markdown transcription of the PDF, adapted for use in code repositories (so GitHub Copilot can index it). Any code fragments are **pseudocode** for structure and not drop-in code.

---

## Read Me First

- **First step:** write your **parser**, finish it completely, and **test it thoroughly** before implementing anything else. Many projects fail because the parser wasn’t done first or carefully.
- **Second step:** implement **semantic error checking**. Do **not** work on Task 2 (execution) or Tasks 3–5 (polynomial rewriting) before finishing semantic checks.
- **Third step:** implement **monomial combining and reordering** (Task 3). This lays the groundwork for Tasks 2, 4, and 5.
- Read this guide at least twice; treat **all code fragments as pseudocode** (they weren’t compiled by the author).

---

## Contents (as listed in the guide)

- Major components — One page  
- Parsing — Two pages  
- Semantic checking — Three pages  
- Program execution — One page  
- Memory allocation for variables — One page  
- Storing inputs — One page  
- Representing polynomial declarations — Four pages  
- Representing the program as a linked list — One page  
- Representing polynomials for evaluation — Two pages  
- Generating a statement list — One page  
- Executing the program — One page  
- Evaluating polynomials — One page  

---

## Major Components

Total is 115% (bonus 15%).

1. **Error Checking (60%)**
   - **Syntax checking (30%)** — Finish the parser first. Test thoroughly.
   - **Semantic error checking (30%)** — Important but not the most involved; start early.

2. **Program Execution (15%)** — Requires supporting functionality:
   1. **Memory allocation** for variables in EXECUTE.
   2. **Storing inputs** in an internal structure.
   3. **Polynomial declaration** representation for later execution.
   4. **INPUT statements** → intermediate representation (IR).
   5. **OUTPUT statements** → IR.
   6. **Assignment statements** → IR.
   7. **Statement list** → linked list (or vector) for later execution.
   8. **Execution** — `execute_program()` consumes the statement list and simulates program execution.

3. **Polynomial Rewriting (40%)**
   - **Task 3:** rewrite **monomial lists** (combine like variables; reorder by parameter order).
   - **Task 4:** combine **identical monomial lists** in a term list (sum coefficients).
   - **Task 5:** **expand** and simplify polynomials using the earlier tasks.

---

## Parsing (1/2)

**Approach:** one parsing function per **nonterminal**. When `parse_X()` is called, it must consume input for `X`. Use lecture material as reference.

Example skeletons (pseudocode — not compiled):

```cpp
void parse_input() {
  // the input consists of a program followed by nothing
  parse_program();
  expect(END_OF_FILE);
}

void parse_program() {
  // program -> tasks_section poly_section execute_section inputs_section
  parse_tasks_section();
  parse_poly_section();
  parse_execute_section();
  parse_inputs_section();
}
```

`program` has one rule: `A → B C D E`. Parsing A means parsing B then C then D then E in order. Same pattern applies to other single-rule nonterminals.

---

## Parsing (2/2)

Example: `poly_decl`

```cpp
void parse_poly_decl() {
  // poly_decl -> poly_header EQUAL poly_body SEMICOLON
  parse_poly_header();
  expect(EQUAL);
  parse_poly_body();
  expect(SEMICOLON);
}
```

- Use `parse_X()` for nonterminals and `expect(TOKEN)` for terminals.
- Good practice: include the grammar rule as a comment atop each parse function to avoid mistakes.

Example with **choice** (lookahead) for `term` (4 rules); use `peek(k)` to decide:

```cpp
void parse_term() {
  // term -> monomial_list
  // term -> coefficient monomial_list
  // term -> coefficient
  // term -> parenthesized_list
  Token t1 = lexer.peek(1);
  if (t1.token_type == ID) {
    parse_monomial_list();
  } else if (t1.token_type == NUM) {
    parse_coefficient();
    Token t2 = lexer.peek(1);
    if (t2.token_type == ID) {
      parse_monomial_list();
    } else {
      /* just coefficient case */
    }
  } else if (t1.token_type == LPAREN) {
    parse_parenthesized_list();
  } else {
    syntax_error();
  }
}
```

> Some rules need `peek(2)`; this grammar occasionally requires two-token lookahead.

---

## Semantic Error Checking (1/3)

You must detect:

1. **Duplicate polynomial declaration** (same name appears in multiple headers).
2. **Invalid monomial name** (ID in body not in parameter list).
3. **Undeclared polynomial used** in evaluation.
4. **Wrong number of arguments** in an evaluation.

To support this, store information about declarations while parsing, then check uses against the stored information.

**Relevant grammar (portion):**
```
poly_section -> POLY poly_decl_list
poly_decl_list -> poly_decl
poly_decl_list -> poly_decl poly_decl_list
poly_decl -> poly_header EQUAL poly_body SEMICOLON
poly_header -> poly_name
poly_header -> poly_name LPAREN id_list RPAREN
id_list -> ID
id_list -> ID COMMA id_list
```

Example `parse_poly_decl_list()` approach:

```cpp
void parse_poly_decl_list() {
  // poly_decl_list -> poly_decl
  // poly_decl_list -> poly_decl poly_decl_list
  parse_poly_decl();        // parses one declaration
  Token t = lexer.peek(1);
  if (t.token_type == ID) { // ID begins the next decl (per this guide’s assumption)
    parse_poly_decl_list();
  } else if (t.token_type == EXECUTE) {
    return; // next section
  } else {
    syntax_error();
  }
}
```

> Note: The `EXECUTE` lookahead can be omitted here and rely on `parse_program()` to proceed into `parse_execute_section()` which will itself `expect(EXECUTE)`.

---

## Semantic Error Checking (2/3)

We now return **structured** info from parse functions.

```cpp
struct poly_header_t {
  Token name;
  vector<Token> id_list; // or strings; default to {"x"} if no explicit list
};

poly_header_t parse_poly_header() {
  poly_header_t header;
  header.name = parse_poly_name();
  Token t = lexer.peek(1);
  if (t.token_type == LPAREN) {
    expect(LPAREN);
    header.id_list = parse_id_list();
    expect(RPAREN);
  } else {
    header.id_list = make_list("x"); // default variable
  }
  return header;
}
```

`poly_decl` returns a compound struct:

```cpp
struct poly_body_t; // defined later
struct poly_decl_t {
  poly_header_t header;
  poly_body_t   body;
};

poly_decl_t parse_poly_decl() {
  poly_decl_t decl;
  decl.header = parse_poly_header();
  expect(EQUAL);
  decl.body = parse_poly_body();
  expect(SEMICOLON);
  return decl;
}
```

---

## Semantic Error Checking (3/3)

Maintain a **global vector** of `poly_decl_t` to store all declarations (flat list).

```cpp
vector<poly_decl_t> polynomials;

void parse_poly_decl_list() {
  poly_decl_t decl = parse_poly_decl();
  polynomials.push_back(decl);  // pushes every decl as recursion unwinds
  Token t = lexer.peek(1);
  if (t.token_type == ID) {
    parse_poly_decl_list();
  }
}
```

- To detect **duplicates**, check `decl.header.name` against existing entries before pushing.
- For **IM-4** (invalid monomial name) while parsing bodies, you need access to the current header’s param list. Options:
  - Pass the header to `parse_poly_body(header)`; or
  - Push a partial `poly_decl_t` (header only) first, so the header is globally accessible while parsing the body.
- For **AUP-13** (undeclared polynomial) and **NA-7** (wrong arity), the declarations table and header `id_list` size provide what you need.

---

## Program Execution — Overview

Task 2 (and Tasks 3–5) require recursive data structures for **polynomial bodies** and a representation of the **EXECUTE** section.

For Task 2 you will:
1. Represent polynomial bodies in a structure usable at runtime.
2. Allocate memory locations for variables used in EXECUTE (for INPUT/assignment/OUTPUT).
3. Represent EXECUTE as a list of statement nodes.
4. After parsing, call `execute_program()` with the statement list.

---

## Allocating Memory for Variables (1/2)

Every variable in EXECUTE needs a fixed location.

Example (pseudocode for INPUT; the idea applies also to assignment LHS, arguments in evaluations, and OUTPUT):

```cpp
stmt_t* parse_input_statement() {
  // input_statement -> INPUT ID SEMICOLON
  auto st = new stmt_t;
  expect(INPUT);
  Token t = expect(ID);
  expect(SEMICOLON);

  // If t.lexeme not in symbol table, allocate:
  //   symbol[t.lexeme] = next_available++;
  // Then, for IR:
  //   st->stmt_type = INPUT;
  //   st->var = symbol[t.lexeme];
  return st;
}
```

**Where to allocate:** on first encounter of a variable name anywhere in EXECUTE (INPUT, OUTPUT, assignment LHS, or as an ID argument).

**How to allocate:** keep `next_available` as a global counter; on first sight of a name, set `symbol[name] = next_available++`.

**Note:** Program variables have memory; **polynomial parameters do not**.

### Image description (symbol table & memory diagram)

- A diagram shows variables `X, Y, Z, W` mapped to locations `0, 1, 2, 3` in a **symbol table** (a map).  
- A vertical **memory** array indexed 0..7 is shown with initial zeros. After some inputs/assignments, some cells hold example values (e.g., `mem[1] = 5`, `mem[2] = 3`).  
- The mapping list shows `"X"→0`, `"Y"→1`, `"Z"→2`, `"W"→3` and memory cells initialized to 0.  

---

## Allocating Memory for Variables (2/2)

A small program illustrates four variables and their assigned locations. The table (map) contains the name→index pairs; memory is a separate integer array. Emphasis: allocate **the first time** each variable is seen; all entries default to zero.

---

## Storing Inputs

- Parse the `INPUTS` section and store values **sequentially** into a vector, e.g., `inputs = [1, 4, 17, 18, 19, 13, 14]`.
- Use `std::stoi()` to convert tokens to integers.
- Maintain an index `next_input` that advances each time an `INPUT` statement executes.

### Image description (inputs vector)

- A vertical list labeled **inputs** with indices `0..6` and values `1, 4, 17, 18, 19, 13, 14`.  
- Side-by-side with the **symbol table** and **memory** diagrams from the previous section for context.

---

## Representing Polynomial Declarations (1/3)

Build the representation in parts:

1. **Header** returns the polynomial name and parameter list (`["x"]` by default if no explicit list).
2. **Coefficient** returns an `int`.
3. **Exponent** returns an `int`.
4. **Monomial** returns `{ ID token, exponent }` (exponent defaults to 1).
5. **Monomial list** builds a **vector<int> powers**, aligned to the **parameter order** (increment positions as monomials are parsed).
6. **Term** returns both coefficient and monomial list (or marks a parenthesized list).
7. **Polynomial body** returns a list of **terms** with attached **operators** (PLUS/MINUS).

Pseudocode sketches:

```cpp
int parse_coefficient() {
  Token t = expect(NUM);
  return std::stoi(t.lexeme);
}

struct monomial_t {
  Token ID;
  int exponent; // default 1 if ^e is absent
};

monomial_t parse_monomial() {
  // parse ID and optional ^ NUM -> fill ID + exponent
}
```

Define a `TermNode` to cover both monomial lists **and** parenthesized lists:

```cpp
enum Kind { MLIST, PARENLIST };
enum Operator { PLUS, MINUS };

struct TermNode {
  Kind kind;                      // MLIST or PARENLIST
  Operator op;                    // operator preceding this term
  int coefficient;                // only for MLIST
  vector<int> monomial_list;      // only for MLIST (powers aligned to params)
  vector<vector<TermNode>> parenthesized_list; // only for PARENLIST
};
```

> Note from guide: this PARENLIST field was mistakenly omitted in the project spec; include it here.

---

## Representing Polynomial Declarations (2/3)

Pseudocode fragments:

```cpp
void parse_monomial_list(vector<int>& powers /*by ref*/) {
  // parse one monomial, locate its variable position using current header params,
  // update powers[pos] += exponent (or +=1 if none),
  // if more monomials follow, recurse while passing powers by reference.
}

void parse_term(TermNode& term) {
  // parse a term, set kind/op, fill coefficient + monomial_list
  // OR fill parenthesized_list (vector of term lists) for ( ... )( ... )...
}

void parse_polynomial_body(vector<TermNode>& termList) {
  // parse a sequence of terms with +/-, append to termList in order
}
```

---

## Representing Polynomial Declarations (3/3) — Examples & Tables

Maintain a **polynomial table** (vector). Each entry holds the header (name + parameters) and the body (vector of terms). When parsing, push the header first (so parameter order is accessible to monomial parsing), then fill the body.

### Example declarations

```
F = x^2 + 5;
G(x,y,z) = 2 y^3 x^4 + 3 x z^2;
H(x,y,z) = (1 + 2 x^3)(4 + 5 z^6) + 7 y^8;
```

#### After parsing `F` (1 parameter `x`)

- **Body** is vector of two MLIST terms:
  - Term 1: `op=PLUS`, `coefficient=1`, `monomial_list=[2]` (x²)
  - Term 2: `op=PLUS`, `coefficient=5`, `monomial_list=[0]` (x⁰)

#### After parsing `G(x,y,z)`

- **Body** MLIST terms with powers aligned to `[x,y,z]` (vector length 3):
  - Term 1: `op=PLUS`, `coefficient=2`, `monomial_list=[4,3,0]` (2·x⁴y³)
  - Term 2: `op=MINUS`, `coefficient=3`, `monomial_list=[1,0,2]` (−3·x·z²)

#### After parsing `H(x,y,z)`

- **Body** has three top-level terms:
  1. PARENLIST (op=PLUS; implicit coefficient 1). Its **parenthesized_list** holds two **term lists** representing `(1 + 2 x^3)` and `(4 + 5 z^6)`. Each inner term list is a small vector of MLIST terms (e.g., `1`, `2 x^3`, etc.).
  2. MLIST (op=PLUS), coefficient `7`, monomial_list `[0,8,0]` (7·y⁸).
  3. PARENLIST (op=MINUS; implicit coefficient 1) for a second product like `(9 + 10 x^11)(12 + 13 z^14)` in the extended example below.

### Image description (tables/blocks)

- A table with columns **Name**, **Parameters**, **Body**.  
- For `F`, **Parameters** shows `"x"`. **Body** shows two MLIST rows with operators and fields (`coefficient`, `powers`).  
- For `G`, **Parameters** shows `"x" "y" "z"`; **Body** shows two MLIST rows with `PLUS` / `MINUS` and power vectors `[4,3,0]` and `[1,0,2]`.  
- For `H`, **Body** shows nested **PARENLIST** containing two boxed **term lists**, each with its own MLIST entries.

Extended example from the guide:

```
F = x^2 + 5;
G(x,y,z) = 2 y^3 x^4 - 3 x z^2;
H(x,y,z) = (1 + 2 x^3)(4 + 5 z^6) + 7 y^8 - (9 + 10 x^11)(12 + 13 z^14);
```

- Representation for `H` shows two PARENLIST terms (product factors) and one MLIST for `+ 7 y^8`.

---

## Representing the Program as a Linked List

Represent the program as a **vector of statements** (or a linked list).

```cpp
struct Stmt_Node_t {
  int stmt_type;         // INPUT, OUTPUT, ASSIGN
  int LHS;               // for ASSIGN: location of LHS variable
  poly_eval_t* poly_eval;// for ASSIGN: pointer to polynomial evaluation IR
  int var;               // for INPUT/OUTPUT: variable location
};
```

Example EXECUTE:

```
INPUT X;
INPUT Y;
Y = F(X,W);
W = F(X,W);
OUTPUT W;
```

- `INPUT` and `OUTPUT` nodes store `var` (location index).
- `ASSIGN` nodes store `LHS` and a pointer to a `poly_eval_t` that describes the right-hand polynomial evaluation IR.

---

## Representing Polynomials for Evaluation

A polynomial evaluation needs:

1. **Which polynomial** to call — index into the declarations table.
2. **Arguments** — each is one of:
   - `NUM`: store the integer value.
   - `ID`: store the variable location index.
   - `POLYEVAL`: recursively store a nested `poly_eval_t*`.

Pseudocode:

```cpp
struct poly_argument_t; // forward decl

struct poly_eval_t {
  int polyIndex;                      // which polynomial
  vector<poly_argument_t> polyArgs;   // one per formal parameter
};

enum ArgKind { ARG_NUM, ARG_ID, ARG_POLYEVAL };

struct poly_argument_t {
  ArgKind kind;
  int value;              // when NUM
  int varIndex;           // when ID
  poly_eval_t* polyEval;  // when POLYEVAL
};
```

### Example

```
INPUT X;
INPUT Y;
Y = F(1);
W = G( G(4, X, Y), 2, 3 );
OUTPUT W;
```

- The `ASSIGN Y = F(1)` node has `polyIndex = index(F)`, `polyArgs = [{NUM, value=1}]`.
- The `ASSIGN W = G(...)` node has `polyIndex = index(G)`, with three arguments:
  1. A nested `POLYEVAL` node for `G(4, X, Y)` → its own 3 arguments: `{NUM 4}`, `{ID X}`, `{ID Y}`.
  2. `{NUM 2}`.
  3. `{NUM 3}`.

### Image description (evaluation trees)

- A tree diagram shows `W = G( … )` at the parent, with a left child that is itself `G(4, X, Y)` and two leaf numeric arguments `2` and `3`. Leaves labeled with `ID` carry a small tag pointing to the variable’s memory index (`X→0`, `Y→1`).

---

## Generating a Statement List

Recursive list construction pattern:

```cpp
void parse_statement_list(vector<Stmt_Node_t>& stmtList) {
  Stmt_Node_t st = parse_stmt();
  stmtList.push_back(st);
  // If more input remains for statements:
  parse_statement_list(stmtList);
}
```

(Use lookahead to know when to stop; e.g., at end of EXECUTE or when another section starts.)

---

## Executing the Program

Parsing builds:
1. **Symbol table** (name → location index).
2. **Polynomial table** (headers + bodies).
3. **Program** as a vector of statement nodes.

Then execute:

```cpp
void execute_program(const vector<Stmt_Node_t>& program) {
  int v;
  for (const auto& stmt : program) {
    switch (stmt.stmt_type) {
      case ASSIGN:
        v = evaluate_polynomial(stmt.poly_eval);
        mem[stmt.LHS] = v;
        break;
      case OUTPUT:
        std::cout << mem[stmt.var] << std::endl;
        break;
      case INPUT:
        mem[stmt.var] = inputs[next_input++];
        break;
    }
  }
}
```

- `next_input` tracks how many input numbers have been consumed.
- `evaluate_polynomial(...)` handles the actual computation (next section).

---

## Evaluating Polynomials

1. **Evaluate arguments** first:
   - `NUM` → use stored value.
   - `ID` → read `mem[varIndex]`.
   - `POLYEVAL` → recursively call `evaluate_polynomial`.
2. With all argument values ready:
   - Evaluate the **polynomial body** using the term representation:
     - For **MLIST** term: compute `coefficient * product(arg[i]^power[i])`.
     - For **PARENLIST**: evaluate each inner **term list** `( … )` to a value; multiply adjacent parenthesized values; that product becomes the term value (preceded by its operator).
   - Sum/subtract across the top-level term list according to each term’s `op`.

> Real compilers execute procedures iteratively with activation records and parameters; here, evaluation is recursive for simplicity.
