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

struct PolynomialDeclaration {
    std::string name;
    std::vector<std::string> parameters;
    int line_no;
    bool has_params;
};

struct PolynomialEvaluation {
    std::string name;
    int arg_count;
    int line_no;
};

struct VariableUsage {
    std::string var_name;
    int line_no;
    std::string poly_name;
};

class Parser {
  public:
    void ParseProgram();

  private:
    LexicalAnalyzer lexer;
    
    // Data structures for semantic analysis
    std::vector<PolynomialDeclaration> polynomial_declarations;
    std::vector<PolynomialEvaluation> polynomial_evaluations;
    std::vector<VariableUsage> variable_usages;
    std::vector<int> task_list;
    
    // Helper functions
    void syntax_error();
    Token expect(TokenType expected_type);
    
    // Grammar parsing functions
    void parse_program();
    void parse_tasks_section();
    void parse_num_list();
    void parse_poly_section();
    void parse_poly_decl_list();
    void parse_poly_decl();
    void parse_poly_header(PolynomialDeclaration& poly_decl);
    void parse_id_list(std::vector<std::string>& params);
    void parse_poly_body(const std::string& poly_name, const std::vector<std::string>& params);
    void parse_term_list(const std::string& poly_name, const std::vector<std::string>& params);
    void parse_term(const std::string& poly_name, const std::vector<std::string>& params);
    void parse_monomial_list(const std::string& poly_name, const std::vector<std::string>& params);
    void parse_monomial(const std::string& poly_name, const std::vector<std::string>& params);
    void parse_exponent();
    void parse_parenthesized_list(const std::string& poly_name, const std::vector<std::string>& params);
    void parse_add_operator();
    void parse_coefficient();
    void parse_execute_section();
    void parse_statement_list();
    void parse_statement();
    void parse_input_statement();
    void parse_output_statement();
    void parse_assign_statement();
    void parse_poly_evaluation();
    void parse_argument_list(int& arg_count);
    void parse_argument();
    void parse_inputs_section();
    
    // Semantic checking functions
    bool check_semantic_errors();
    bool check_duplicate_polynomials();
    bool check_invalid_monomial_names();
    bool check_undeclared_polynomials();
    bool check_wrong_argument_count();
    
    // Utility functions
    bool is_task_listed(int task_num);
    void output_semantic_error(const std::string& error_code, const std::vector<int>& line_numbers);
};

#endif

