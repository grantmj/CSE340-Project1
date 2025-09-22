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

// Data structures for polynomial representation
struct Monomial {
    std::string var_name;
    int exponent;
};

struct Term {
    int coefficient;
    std::vector<Monomial> monomials;
    bool is_positive; // for handling +/- operators
    bool is_first_term; // to handle printing
};

struct PolyBody {
    std::vector<Term> terms;
};

struct PolyDecl {
    std::string name;
    std::vector<std::string> params;
    int line_number;
    PolyBody body; // Add body representation
};

class Parser {
  public:
    void parse_program();

  private:
    LexicalAnalyzer lexer;
    void syntax_error();
    Token expect(TokenType expected_type);
    
    // Data structures for semantic checking
    std::vector<PolyDecl> polynomials;
    std::map<std::string, std::vector<int>> duplicate_lines; // name -> line numbers
    std::vector<int> im4_errors;  // line numbers for IM-4 errors
    std::vector<int> aup13_errors; // line numbers for AUP-13 errors  
    std::vector<int> na7_errors;  // line numbers for NA-7 errors
    PolyDecl* current_poly; // for checking monomial names during body parsing
    bool has_semantic_errors;
    
    // Task tracking
    std::vector<int> task_numbers;
    
    // Task 3 specific data
    Term* current_term; // for building terms during parsing
    
    // Semantic checking functions
    void check_semantic_errors();
    void output_semantic_errors();
    bool is_valid_monomial(const std::string& name);
    PolyDecl* find_polynomial(const std::string& name);
    
    // Task 3 functions
    void execute_tasks();
    void execute_task3();
    std::vector<int> convert_to_power_array(const std::vector<Monomial>& monomials, const std::vector<std::string>& params);
    std::string format_monomial_list(const std::vector<int>& power_array, const std::vector<std::string>& params);
    void print_polynomial_with_sorted_monomials(const PolyDecl& poly);
    
    // Parsing functions for each nonterminal
    void parse_tasks_section();
    void parse_num_list();
    void parse_poly_section();
    void parse_poly_decl_list();
    void parse_poly_decl();
    void parse_poly_header();
    void parse_id_list();
    std::vector<std::string> parse_id_list_return(); // returns vector of param names
    void parse_poly_name();
    void parse_poly_body();
    void parse_term_list();
    void parse_term_list_with_operator(bool is_negative); // helper for handling operators
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
};

#endif
