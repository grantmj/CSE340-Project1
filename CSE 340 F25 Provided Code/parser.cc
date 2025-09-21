/*
 * Copyright (C) Rida Bazzi, 2020
 *
 * Do not share this file with anyone
 *
 * Do not post this file or derivatives of
 * of this file online
 *
 */
#include <iostream>
#include <cstdlib>
#include <algorithm>
#include "parser.h"

using namespace std;

// You should provide the syntax error message for this function
void Parser::syntax_error()
{
    cout << "SYNTAX ERROR !!!!!&%!!!!&%!!!!!!" << endl;
    exit(1);
}

// this function gets a token and checks if it is
// of the expected type. If it is, the token is
// returned, otherwise, synatx_error() is generated
// this function is particularly useful to match
// terminals in a right hand side of a rule.
// Written by Mohsen Zohrevandi
Token Parser::expect(TokenType expected_type)
{
    Token t = lexer.GetToken();
    if (t.token_type != expected_type)
        syntax_error();
    return t;
}

void Parser::ParseProgram()
{
    parse_program();
    
    // Check for semantic errors
    bool has_semantic_error = check_semantic_errors();
    
    // If task 1 is listed and there are semantic errors, we already output and exited
    // If task 1 is not listed and there are semantic errors, output nothing and exit
    if (has_semantic_error && !is_task_listed(1)) {
        exit(0);
    }
    
    // If we reach here, there are no errors
    // Execute other tasks based on task_list (not implemented yet)
}

// Grammar parsing functions
void Parser::parse_program()
{
    parse_tasks_section();
    parse_poly_section();
    parse_execute_section();
    parse_inputs_section();
}

void Parser::parse_tasks_section()
{
    expect(TASKS);
    parse_num_list();
}

void Parser::parse_num_list()
{
    Token num_token = expect(NUM);
    int task_num = stoi(num_token.lexeme);
    task_list.push_back(task_num);
    
    Token t = lexer.peek(1);
    if (t.token_type == NUM) {
        parse_num_list();
    }
}

void Parser::parse_poly_section()
{
    expect(POLY);
    parse_poly_decl_list();
}

void Parser::parse_poly_decl_list()
{
    parse_poly_decl();
    
    Token t = lexer.peek(1);
    if (t.token_type == ID) {
        parse_poly_decl_list();
    }
}

void Parser::parse_poly_decl()
{
    PolynomialDeclaration poly_decl;
    parse_poly_header(poly_decl);
    polynomial_declarations.push_back(poly_decl);
    
    expect(EQUAL);
    parse_poly_body(poly_decl.name, poly_decl.parameters);
    expect(SEMICOLON);
}

void Parser::parse_poly_header(PolynomialDeclaration& poly_decl)
{
    Token poly_name = expect(ID);
    poly_decl.name = poly_name.lexeme;
    poly_decl.line_no = poly_name.line_no;
    poly_decl.has_params = false;
    
    Token t = lexer.peek(1);
    if (t.token_type == LPAREN) {
        expect(LPAREN);
        poly_decl.has_params = true;
        parse_id_list(poly_decl.parameters);
        expect(RPAREN);
    }
}

void Parser::parse_id_list(vector<string>& params)
{
    Token id = expect(ID);
    params.push_back(id.lexeme);
    
    Token t = lexer.peek(1);
    if (t.token_type == COMMA) {
        expect(COMMA);
        parse_id_list(params);
    }
}

void Parser::parse_poly_body(const string& poly_name, const vector<string>& params)
{
    parse_term_list(poly_name, params);
}

void Parser::parse_term_list(const string& poly_name, const vector<string>& params)
{
    parse_term(poly_name, params);
    
    Token t = lexer.peek(1);
    if (t.token_type == PLUS || t.token_type == MINUS) {
        parse_add_operator();
        parse_term_list(poly_name, params);
    }
}

void Parser::parse_term(const string& poly_name, const vector<string>& params)
{
    Token t = lexer.peek(1);
    
    if (t.token_type == LPAREN) {
        parse_parenthesized_list(poly_name, params);
    } else if (t.token_type == NUM) {
        Token num_lookahead = lexer.peek(2);
        if (num_lookahead.token_type == ID || num_lookahead.token_type == SEMICOLON || 
            num_lookahead.token_type == PLUS || num_lookahead.token_type == MINUS ||
            num_lookahead.token_type == RPAREN) {
            parse_coefficient();
            Token next = lexer.peek(1);
            if (next.token_type == ID) {
                parse_monomial_list(poly_name, params);
            }
        } else {
            parse_coefficient();
        }
    } else if (t.token_type == ID) {
        parse_monomial_list(poly_name, params);
    }
}

void Parser::parse_monomial_list(const string& poly_name, const vector<string>& params)
{
    parse_monomial(poly_name, params);
    
    Token t = lexer.peek(1);
    if (t.token_type == ID) {
        parse_monomial_list(poly_name, params);
    }
}

void Parser::parse_monomial(const string& poly_name, const vector<string>& params)
{
    Token id = expect(ID);
    
    // Record variable usage for semantic checking
    VariableUsage usage;
    usage.var_name = id.lexeme;
    usage.line_no = id.line_no;
    usage.poly_name = poly_name;
    variable_usages.push_back(usage);
    
    Token t = lexer.peek(1);
    if (t.token_type == POWER) {
        parse_exponent();
    }
}

void Parser::parse_exponent()
{
    expect(POWER);
    expect(NUM);
}

void Parser::parse_parenthesized_list(const string& poly_name, const vector<string>& params)
{
    expect(LPAREN);
    parse_term_list(poly_name, params);
    expect(RPAREN);
    
    Token t = lexer.peek(1);
    if (t.token_type == LPAREN) {
        parse_parenthesized_list(poly_name, params);
    }
}

void Parser::parse_add_operator()
{
    Token t = lexer.peek(1);
    if (t.token_type == PLUS) {
        expect(PLUS);
    } else if (t.token_type == MINUS) {
        expect(MINUS);
    } else {
        syntax_error();
    }
}

void Parser::parse_coefficient()
{
    expect(NUM);
}

void Parser::parse_execute_section()
{
    expect(EXECUTE);
    parse_statement_list();
}

void Parser::parse_statement_list()
{
    parse_statement();
    
    Token t = lexer.peek(1);
    if (t.token_type == INPUT || t.token_type == OUTPUT || t.token_type == ID) {
        parse_statement_list();
    }
}

void Parser::parse_statement()
{
    Token t = lexer.peek(1);
    
    if (t.token_type == INPUT) {
        parse_input_statement();
    } else if (t.token_type == OUTPUT) {
        parse_output_statement();
    } else if (t.token_type == ID) {
        parse_assign_statement();
    } else {
        syntax_error();
    }
}

void Parser::parse_input_statement()
{
    expect(INPUT);
    expect(ID);
    expect(SEMICOLON);
}

void Parser::parse_output_statement()
{
    expect(OUTPUT);
    expect(ID);
    expect(SEMICOLON);
}

void Parser::parse_assign_statement()
{
    expect(ID);
    expect(EQUAL);
    parse_poly_evaluation();
    expect(SEMICOLON);
}

void Parser::parse_poly_evaluation()
{
    Token poly_name = expect(ID);
    expect(LPAREN);
    
    PolynomialEvaluation eval;
    eval.name = poly_name.lexeme;
    eval.line_no = poly_name.line_no;
    eval.arg_count = 0;
    
    parse_argument_list(eval.arg_count);
    polynomial_evaluations.push_back(eval);
    
    expect(RPAREN);
}

void Parser::parse_argument_list(int& arg_count)
{
    parse_argument();
    arg_count++;
    
    Token t = lexer.peek(1);
    if (t.token_type == COMMA) {
        expect(COMMA);
        parse_argument_list(arg_count);
    }
}

void Parser::parse_argument()
{
    Token t = lexer.peek(1);
    
    if (t.token_type == ID) {
        Token id = expect(ID);
        Token next = lexer.peek(1);
        if (next.token_type == LPAREN) {
            // This is a polynomial evaluation as an argument
            expect(LPAREN);
            int nested_arg_count = 0;
            parse_argument_list(nested_arg_count);
            expect(RPAREN);
            
            // Record this evaluation too
            PolynomialEvaluation nested_eval;
            nested_eval.name = id.lexeme;
            nested_eval.line_no = id.line_no;
            nested_eval.arg_count = nested_arg_count;
            polynomial_evaluations.push_back(nested_eval);
        }
    } else if (t.token_type == NUM) {
        expect(NUM);
    } else {
        syntax_error();
    }
}

void Parser::parse_inputs_section()
{
    expect(INPUTS);
    parse_num_list();
}

// Semantic checking functions
bool Parser::check_semantic_errors()
{
    // Check each type of semantic error in order
    if (check_duplicate_polynomials()) return true;
    if (check_invalid_monomial_names()) return true;
    if (check_undeclared_polynomials()) return true;
    if (check_wrong_argument_count()) return true;
    
    return false;
}

bool Parser::check_duplicate_polynomials()
{
    map<string, vector<int>> poly_lines;
    
    // Collect all occurrences of each polynomial name
    for (const auto& poly : polynomial_declarations) {
        poly_lines[poly.name].push_back(poly.line_no);
    }
    
    // Find duplicates
    vector<int> duplicate_lines;
    for (const auto& entry : poly_lines) {
        if (entry.second.size() > 1) {
            // Add all occurrences except the first one
            for (size_t i = 1; i < entry.second.size(); i++) {
                duplicate_lines.push_back(entry.second[i]);
            }
        }
    }
    
    if (!duplicate_lines.empty()) {
        sort(duplicate_lines.begin(), duplicate_lines.end());
        if (is_task_listed(1)) {
            output_semantic_error("DMT-12", duplicate_lines);
            exit(1);
        }
        return true;
    }
    
    return false;
}

bool Parser::check_invalid_monomial_names()
{
    vector<int> error_lines;
    
    for (const auto& usage : variable_usages) {
        // Find the polynomial declaration
        PolynomialDeclaration* poly_decl = nullptr;
        for (auto& decl : polynomial_declarations) {
            if (decl.name == usage.poly_name) {
                poly_decl = &decl;
                break;
            }
        }
        
        if (poly_decl) {
            bool valid = false;
            
            if (!poly_decl->has_params) {
                // Polynomial without parameters - only 'x' is valid
                if (usage.var_name == "x") {
                    valid = true;
                }
            } else {
                // Polynomial with parameters - variable must be in parameter list
                for (const string& param : poly_decl->parameters) {
                    if (usage.var_name == param) {
                        valid = true;
                        break;
                    }
                }
            }
            
            if (!valid) {
                error_lines.push_back(usage.line_no);
            }
        }
    }
    
    if (!error_lines.empty()) {
        sort(error_lines.begin(), error_lines.end());
        if (is_task_listed(1)) {
            output_semantic_error("IM-4", error_lines);
            exit(1);
        }
        return true;
    }
    
    return false;
}

bool Parser::check_undeclared_polynomials()
{
    vector<int> error_lines;
    
    for (const auto& eval : polynomial_evaluations) {
        bool found = false;
        for (const auto& decl : polynomial_declarations) {
            if (decl.name == eval.name) {
                found = true;
                break;
            }
        }
        
        if (!found) {
            error_lines.push_back(eval.line_no);
        }
    }
    
    if (!error_lines.empty()) {
        sort(error_lines.begin(), error_lines.end());
        if (is_task_listed(1)) {
            output_semantic_error("AUP-13", error_lines);
            exit(1);
        }
        return true;
    }
    
    return false;
}

bool Parser::check_wrong_argument_count()
{
    vector<int> error_lines;
    
    for (const auto& eval : polynomial_evaluations) {
        // Find the polynomial declaration
        for (const auto& decl : polynomial_declarations) {
            if (decl.name == eval.name) {
                int expected_args = decl.has_params ? decl.parameters.size() : 1;
                if (eval.arg_count != expected_args) {
                    error_lines.push_back(eval.line_no);
                }
                break;
            }
        }
    }
    
    if (!error_lines.empty()) {
        sort(error_lines.begin(), error_lines.end());
        if (is_task_listed(1)) {
            output_semantic_error("NA-7", error_lines);
            exit(1);
        }
        return true;
    }
    
    return false;
}

// Utility functions
bool Parser::is_task_listed(int task_num)
{
    for (int task : task_list) {
        if (task == task_num) {
            return true;
        }
    }
    return false;
}

void Parser::output_semantic_error(const string& error_code, const vector<int>& line_numbers)
{
    cout << "Semantic Error Code " << error_code << ":";
    for (size_t i = 0; i < line_numbers.size(); i++) {
        cout << " " << line_numbers[i];
    }
    cout << " " << endl;
}

int main()
{
    // note: the parser class has a lexer object instantiated in it. You should not be declaring
    // a separate lexer object. You can access the lexer object in the parser functions as shown in the
    // example method Parser::ConsumeAllInput
    // If you declare another lexer object, lexical analysis will not work correctly
    Parser parser;

    parser.ParseProgram();
    return 0;
}
