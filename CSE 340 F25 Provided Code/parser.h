/*
 * Copyright (C) Rida Bazzi, 2019
 *
 * Do not share this file with anyone
 */
#ifndef __PARSER_H__
#define __PARSER_H__

#include <string>
#include <vector>
#include <map>
#include <set>
#include "lexer.h"

// Simple polynomial declaration for semantic checking
struct PolyDecl {
    std::string name;
    std::vector<std::string> params;
    int line_number;
};

// Rich data structures for polynomial representation (Task 3+)
enum TermKind { MLIST, PARENLIST };
enum OpType { OP_PLUS, OP_MINUS };

// Data structures for Task 2 (program execution)
enum StmtType { STMT_INPUT, STMT_OUTPUT, STMT_ASSIGN };
enum ArgKind { ARG_NUM, ARG_ID, ARG_POLYEVAL };

struct PolyArgument {
    ArgKind kind;
    int value;          // for ARG_NUM
    int var_index;      // for ARG_ID
    struct PolyEval* poly_eval; // for ARG_POLYEVAL
    
    PolyArgument() : kind(ARG_NUM), value(0), var_index(-1), poly_eval(nullptr) {}
};

struct PolyEval {
    int poly_index;                  // index into rich_polynomials
    std::vector<PolyArgument> args;
    
    PolyEval() : poly_index(-1) {}
};

struct Statement {
    StmtType type;
    int var_index;      // for INPUT/OUTPUT: variable location
    int lhs_index;      // for ASSIGN: LHS variable location
    PolyEval* rhs_eval; // for ASSIGN: RHS polynomial evaluation
    
    Statement() : type(STMT_INPUT), var_index(-1), lhs_index(-1), rhs_eval(nullptr) {}
};

struct TermNode {
    TermKind kind;                     // MLIST or PARENLIST
    OpType op;                         // operator preceding this term
    int coefficient;                   // only for MLIST (default 1)
    std::vector<int> monomial_list;    // only for MLIST (powers aligned to params)
    std::vector<std::vector<TermNode>> parenthesized_list; // only for PARENLIST
    
    TermNode() : kind(MLIST), op(OP_PLUS), coefficient(1) {}
};

struct RichPolyDecl {
    std::string name;
    std::vector<std::string> params;
    std::vector<TermNode> body;
    int line_number;
    bool has_explicit_params;  // true if parameters were explicitly specified with parentheses
};

class Parser {
  public:
    void parse_program();

  private:
    LexicalAnalyzer lexer;
    void syntax_error();
    Token expect(TokenType expected_type);
    
    // Task tracking
    std::set<int> requested_tasks;
    
    // Data structures for semantic checking
    std::vector<PolyDecl> polynomials;
    std::map<std::string, std::vector<int>> duplicate_lines; // name -> line numbers
    std::vector<int> im4_errors;  // line numbers for IM-4 errors
    std::vector<int> aup13_errors; // line numbers for AUP-13 errors  
    std::vector<int> na7_errors;  // line numbers for NA-7 errors
    PolyDecl* current_poly; // for checking monomial names during body parsing
    bool has_semantic_errors;
    
    // Rich polynomial representation for Task 3+
    std::vector<RichPolyDecl> rich_polynomials;
    RichPolyDecl* current_rich_poly;
    
    // Task 2 - Program execution data structures
    std::vector<Statement> program;           // list of statements to execute
    std::map<std::string, int> symbol_table;  // variable name -> memory index
    std::vector<int> memory;                  // memory for variables (mem[])
    std::vector<int> inputs;                  // input values from INPUTS section
    int next_input;                           // index of next input to read
    int next_location;                        // next available memory location
    
    // Semantic checking functions
    void check_semantic_errors();
    void output_semantic_errors();
    bool is_valid_monomial(const std::string& name);
    PolyDecl* find_polynomial(const std::string& name);
    
    // Task execution functions
    void execute_tasks();
    void execute_task_2(); // Program execution
    void execute_task_3(); // Sort and combine monomials
    void execute_task_4(); // Combine identical monomial lists
    void execute_task_5(); // Polynomial expansion and simplification
    
    // Task 2 helper functions
    int get_or_create_variable(const std::string& name);
    int evaluate_polynomial(const PolyEval* eval);
    int evaluate_argument(const PolyArgument& arg);
    int evaluate_term(const TermNode& term, const std::vector<int>& arg_values);
    int int_power(int base, int exp);
    PolyEval* parse_poly_evaluation_return();
    PolyArgument parse_argument_return();
    void parse_argument_list_return(std::vector<PolyArgument>& args);
    
    // Task 4 helper functions
    std::vector<TermNode> combine_identical_monomials(const std::vector<TermNode>& terms, const std::vector<std::string>& params);
    bool monomials_are_identical(const std::vector<int>& mono1, const std::vector<int>& mono2);
    
    // Task 5 helper functions
    std::vector<TermNode> expand_polynomial(const std::vector<TermNode>& terms, const std::vector<std::string>& params);
    std::vector<TermNode> expand_parenthesized_term(const TermNode& paren_term, const std::vector<std::string>& params);
    std::vector<TermNode> multiply_term_lists(const std::vector<TermNode>& list1, const std::vector<TermNode>& list2, const std::vector<std::string>& params);
    
    // Parsing functions for each nonterminal
    void parse_tasks_section();
    void parse_num_list();
    void parse_inputs_num_list(); // separate function for INPUTS section
    void parse_poly_section();
    void parse_poly_decl_list();
    void parse_poly_decl();
    void parse_poly_header();
    void parse_id_list();
    std::vector<std::string> parse_id_list_return(); // returns vector of param names
    void parse_poly_name();
    void parse_poly_body();
    void parse_term_list();
    void parse_term();
    void parse_monomial_list();
    void parse_monomial();
    void parse_exponent();
    void parse_parenthesized_list();
    void parse_add_operator();
    void parse_coefficient();
    void parse_execute_section();
    void parse_statement_list();
    void parse_statement();
    void parse_input_statement();
    void parse_output_statement();
    void parse_assign_statement();
    void parse_poly_evaluation();
    void parse_argument_list();
    int parse_argument_list_count(); // counts arguments while parsing
    void parse_argument();
    void parse_inputs_section();
    
    // Rich parsing functions for Tasks 3+
    void parse_rich_poly_body(std::vector<TermNode>& terms);
    void parse_rich_term_list(std::vector<TermNode>& terms);
    void parse_rich_term(TermNode& term);
    void parse_rich_monomial_list(std::vector<int>& powers);
    int parse_rich_coefficient();
    void parse_rich_parenthesized_list(std::vector<std::vector<TermNode>>& paren_list);
    
    // Helper functions for Task 3
    void combine_and_sort_monomials();
    void print_task3_output();
    std::string format_monomial_list(const std::vector<int>& powers, const std::vector<std::string>& params);
    std::string format_term(const TermNode& term, const std::vector<std::string>& params, bool is_first);
    std::string format_poly_decl(const RichPolyDecl& poly);
    std::string format_parenthesized_list(const std::vector<std::vector<TermNode>>& paren_list, const std::vector<std::string>& params);
};

#endif