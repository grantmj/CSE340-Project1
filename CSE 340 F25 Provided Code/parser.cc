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
#include <string>
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

// Parsing

// program → tasks_section poly_section execute_section inputs_section
void Parser::parse_program()
{
    // Initialize semantic checking variables
    polynomials.clear();
    duplicate_lines.clear();
    im4_errors.clear();
    aup13_errors.clear();
    na7_errors.clear();
    current_poly = nullptr;
    current_term = nullptr;
    has_semantic_errors = false;
    task_numbers.clear();
    
    // Initialize Task 2 variables
    program_statements.clear();
    symbol_table.clear();
    memory.clear();
    input_values.clear();
    next_input_index = 0;
    next_memory_location = 0;
    
    parse_tasks_section();
    parse_poly_section();
    parse_execute_section();
    parse_inputs_section();
    expect(END_OF_FILE);
    
    // Check for semantic errors and output if found
    check_semantic_errors();
    if (has_semantic_errors) {
        output_semantic_errors();
    } else {
        // Execute tasks if no errors
        execute_tasks();
    }
}

// tasks_section → TASKS num_list
void Parser::parse_tasks_section()
{
    expect(TASKS);
    parse_num_list();
}

// num_list → NUM
// num_list → NUM num_list
void Parser::parse_num_list()
{
    Token num_token = expect(NUM);
    int task_num = stoi(num_token.lexeme);
    
    // Add task number if not already present
    if (find(task_numbers.begin(), task_numbers.end(), task_num) == task_numbers.end()) {
        task_numbers.push_back(task_num);
    }
    
    Token t = lexer.peek(1);
    if (t.token_type == NUM) {
        parse_num_list();
    }
}

// poly_section → POLY poly_decl_list
void Parser::parse_poly_section()
{
    expect(POLY);
    parse_poly_decl_list();
}

// poly_decl_list → poly_decl
// poly_decl_list → poly_decl poly_decl_list
void Parser::parse_poly_decl_list()
{
    parse_poly_decl();
    Token t = lexer.peek(1);
    if (t.token_type == ID) {
        parse_poly_decl_list();
    }
}

// poly_decl → poly_header EQUAL poly_body SEMICOLON
void Parser::parse_poly_decl()
{
    parse_poly_header();
    expect(EQUAL);
    
    // Initialize current polynomial's body for parsing
    current_poly->body.terms.clear();
    
    parse_poly_body();
    expect(SEMICOLON);
    
    current_poly = nullptr; // Clear current poly after parsing
}

// poly_header → poly_name
// poly_header → poly_name LPAREN id_list RPAREN
void Parser::parse_poly_header()
{
    Token name_token = lexer.peek(1);
    parse_poly_name();
    
    PolyDecl poly;
    poly.name = name_token.lexeme;
    poly.line_number = name_token.line_no;
    
    Token t = lexer.peek(1);
    if (t.token_type == LPAREN) {
        expect(LPAREN);
        // Parse parameter list and store in poly.params
        poly.params = parse_id_list_return();
        expect(RPAREN);
    } else {
        // Default parameter is "x"
        poly.params.push_back("x");
    }
    
    // Check for duplicates
    duplicate_lines[poly.name].push_back(poly.line_number);
    
    polynomials.push_back(poly);
    current_poly = &polynomials.back(); // Set current poly for body parsing
}

// id_list → ID
// id_list → ID COMMA id_list
void Parser::parse_id_list()
{
    expect(ID);
    Token t = lexer.peek(1);
    if (t.token_type == COMMA) {
        expect(COMMA);
        parse_id_list();
    }
}

// Helper function that returns the parameter list as a vector
std::vector<std::string> Parser::parse_id_list_return()
{
    std::vector<std::string> params;
    Token id_token = expect(ID);
    params.push_back(id_token.lexeme);
    
    Token t = lexer.peek(1);
    if (t.token_type == COMMA) {
        expect(COMMA);
        std::vector<std::string> rest = parse_id_list_return();
        params.insert(params.end(), rest.begin(), rest.end());
    }
    return params;
}

// poly_name → ID
void Parser::parse_poly_name()
{
    expect(ID);
}

// poly_body → term_list
void Parser::parse_poly_body()
{
    parse_term_list();
}

// term_list → term
// term_list → term add_operator term_list
void Parser::parse_term_list()
{
    parse_term();
    Token t = lexer.peek(1);
    if (t.token_type == PLUS || t.token_type == MINUS) {
        Token op_token = lexer.GetToken(); // consume operator
        
        // Parse remaining terms and apply operator
        parse_term_list_with_operator(op_token.token_type == MINUS);
    }
}

void Parser::parse_term_list_with_operator(bool is_negative)
{
    // Parse the next term and apply the operator to it
    Term term;
    term.coefficient = 1; // default coefficient
    term.is_positive = !is_negative; // apply the operator
    term.monomials.clear();
    
    Token t1 = lexer.peek(1);
    if (t1.token_type == ID) {
        current_term = &term;
        parse_monomial_list();
    } else if (t1.token_type == NUM) {
        Token coeff_token = expect(NUM);
        term.coefficient = stoi(coeff_token.lexeme);
        
        Token t2 = lexer.peek(1);
        if (t2.token_type == ID) {
            current_term = &term;
            parse_monomial_list();
        }
        // else just coefficient case
    } else if (t1.token_type == LPAREN) {
        // For now, skip parenthesized lists in Task 3
        parse_parenthesized_list();
        return; // Don't add this term yet
    } else {
        syntax_error();
    }
    
    // Add the completed term to current polynomial
    if (current_poly) {
        current_poly->body.terms.push_back(term);
    }
    current_term = nullptr;
    
    // Continue parsing if there are more operators
    Token t = lexer.peek(1);
    if (t.token_type == PLUS || t.token_type == MINUS) {
        Token op_token = lexer.GetToken(); // consume operator
        parse_term_list_with_operator(op_token.token_type == MINUS);
    }
}

// term → monomial_list
// term → coefficient monomial_list
// term → coefficient
// term → parenthesized_list
void Parser::parse_term()
{
    // Create new term
    Term term;
    term.coefficient = 1; // default coefficient
    term.is_positive = true; // will be handled by operator precedence
    term.monomials.clear();
    
    Token t1 = lexer.peek(1);
    if (t1.token_type == ID) {
        current_term = &term;
        parse_monomial_list();
    } else if (t1.token_type == NUM) {
        Token coeff_token = expect(NUM);
        term.coefficient = stoi(coeff_token.lexeme);
        
        Token t2 = lexer.peek(1);
        if (t2.token_type == ID) {
            current_term = &term;
            parse_monomial_list();
        }
        // else just coefficient case
    } else if (t1.token_type == LPAREN) {
        // For now, skip parenthesized lists in Task 3
        parse_parenthesized_list();
        return; // Don't add this term yet
    } else {
        syntax_error();
    }
    
    // Add the completed term to current polynomial
    if (current_poly) {
        current_poly->body.terms.push_back(term);
    }
    current_term = nullptr;
}

// monomial_list → monomial
// monomial_list → monomial monomial_list
void Parser::parse_monomial_list()
{
    parse_monomial();
    Token t = lexer.peek(1);
    if (t.token_type == ID) {
        parse_monomial_list();
    }
}

// monomial → ID
// monomial → ID exponent
void Parser::parse_monomial()
{
    Token id_token = expect(ID);
    
    // Check if this monomial name is valid (IM-4 check)
    if (current_poly && !is_valid_monomial(id_token.lexeme)) {
        im4_errors.push_back(id_token.line_no);
    }
    
    // Create monomial and add to current term
    Monomial monomial;
    monomial.var_name = id_token.lexeme;
    monomial.exponent = 1; // default exponent
    
    Token t = lexer.peek(1);
    if (t.token_type == POWER) {
        expect(POWER);
        Token exp_token = expect(NUM);
        monomial.exponent = stoi(exp_token.lexeme);
    }
    
    // Add monomial to current term
    if (current_term) {
        current_term->monomials.push_back(monomial);
    }
}

// exponent → POWER NUM
void Parser::parse_exponent()
{
    expect(POWER);
    expect(NUM);
}

// parenthesized_list → LPAREN term_list RPAREN
// parenthesized_list → LPAREN term_list RPAREN parenthesized_list
void Parser::parse_parenthesized_list()
{
    expect(LPAREN);
    parse_term_list();
    expect(RPAREN);
    Token t = lexer.peek(1);
    if (t.token_type == LPAREN) {
        parse_parenthesized_list();
    }
}

// add_operator → PLUS
// add_operator → MINUS
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

// coefficient → NUM
void Parser::parse_coefficient()
{
    expect(NUM);
}

// execute_section → EXECUTE statement_list
void Parser::parse_execute_section()
{
    expect(EXECUTE);
    parse_statement_list();
}

// statement_list → statement
// statement_list → statement statement_list
void Parser::parse_statement_list()
{
    parse_statement();
    Token t = lexer.peek(1);
    if (t.token_type == INPUT || t.token_type == OUTPUT || t.token_type == ID) {
        parse_statement_list();
    }
}

// statement → input_statement
// statement → output_statement
// statement → assign_statement
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

// input_statement → INPUT ID SEMICOLON
void Parser::parse_input_statement()
{
    expect(INPUT);
    Token id_token = expect(ID);
    expect(SEMICOLON);
    
    // Task 2: Record the INPUT statement
    Statement stmt;
    stmt.type = INPUT_STMT;
    stmt.var_name = id_token.lexeme;
    stmt.var_location = get_variable_location(id_token.lexeme);
    stmt.poly_eval = nullptr;
    program_statements.push_back(stmt);
}

// output_statement → OUTPUT ID SEMICOLON | OUTPUT poly_evaluation SEMICOLON
void Parser::parse_output_statement()
{
    expect(OUTPUT);
    
    // Check if this is a simple ID or a polynomial evaluation
    Token t1 = lexer.peek(1);
    Token t2 = lexer.peek(2);
    
    if (t1.token_type == ID && t2.token_type == LPAREN) {
        // This is OUTPUT poly_evaluation SEMICOLON
        PolyEvaluation* poly_eval = parse_poly_evaluation_return();
        expect(SEMICOLON);
        
        Statement stmt;
        stmt.type = OUTPUT_STMT;
        stmt.var_name = "";
        stmt.var_location = -1;
        stmt.poly_eval = poly_eval;
        program_statements.push_back(stmt);
    } else if (t1.token_type == ID && t2.token_type == SEMICOLON) {
        // This is OUTPUT ID SEMICOLON
        Token id_token = expect(ID);
        expect(SEMICOLON);
        
        Statement stmt;
        stmt.type = OUTPUT_STMT;
        stmt.var_name = id_token.lexeme;
        stmt.var_location = get_variable_location(id_token.lexeme);
        stmt.poly_eval = nullptr;
        program_statements.push_back(stmt);
    } else {
        syntax_error();
    }
}

// assign_statement → ID EQUAL poly_evaluation SEMICOLON
void Parser::parse_assign_statement()
{
    Token id_token = expect(ID);
    expect(EQUAL);
    
    // Parse polynomial evaluation and save for Task 2
    PolyEvaluation* poly_eval = parse_poly_evaluation_return();
    
    expect(SEMICOLON);
    
    // Task 2: Record the ASSIGN statement
    Statement stmt;
    stmt.type = ASSIGN_STMT;
    stmt.var_name = id_token.lexeme;
    stmt.var_location = get_variable_location(id_token.lexeme);
    stmt.poly_eval = poly_eval;
    program_statements.push_back(stmt);
}

// poly_evaluation → poly_name LPAREN argument_list RPAREN
void Parser::parse_poly_evaluation()
{
    Token name_token = lexer.peek(1);
    parse_poly_name();
    expect(LPAREN);
    
    // Count arguments
    int arg_count = parse_argument_list_count();
    
    expect(RPAREN);
    
    // Check AUP-13: undeclared polynomial
    PolyDecl* poly = find_polynomial(name_token.lexeme);
    if (!poly) {
        aup13_errors.push_back(name_token.line_no);
    } else {
        // Check NA-7: wrong number of arguments
        if (arg_count != (int)poly->params.size()) {
            na7_errors.push_back(name_token.line_no);
        }
    }
}

// argument_list → argument
// argument_list → argument COMMA argument_list
void Parser::parse_argument_list()
{
    parse_argument();
    Token t = lexer.peek(1);
    if (t.token_type == COMMA) {
        expect(COMMA);
        parse_argument_list();
    }
}

// Helper function that counts arguments while parsing
int Parser::parse_argument_list_count()
{
    parse_argument();
    int count = 1;
    Token t = lexer.peek(1);
    if (t.token_type == COMMA) {
        expect(COMMA);
        count += parse_argument_list_count();
    }
    return count;
}

// argument → ID
// argument → NUM
// argument → poly_evaluation
void Parser::parse_argument()
{
    Token t1 = lexer.peek(1);
    if (t1.token_type == ID) {
        Token t2 = lexer.peek(2);
        if (t2.token_type == LPAREN) {
            parse_poly_evaluation();
        } else {
            expect(ID);
        }
    } else if (t1.token_type == NUM) {
        expect(NUM);
    } else {
        syntax_error();
    }
}

// inputs_section → INPUTS num_list
void Parser::parse_inputs_section()
{
    expect(INPUTS);
    parse_num_list_for_inputs();
}

// num_list for inputs - stores values for Task 2
void Parser::parse_num_list_for_inputs()
{
    Token num_token = expect(NUM);
    input_values.push_back(stoi(num_token.lexeme));
    
    Token t = lexer.peek(1);
    if (t.token_type == NUM) {
        parse_num_list_for_inputs();
    }
}

// Semantic checking functions
void Parser::check_semantic_errors()
{
    // Check for duplicate polynomial declarations (DMT-12)
    for (auto& pair : duplicate_lines) {
        if (pair.second.size() > 1) {
            has_semantic_errors = true;
            break;
        }
    }
    
    // Check for other semantic errors
    if (!im4_errors.empty() || !aup13_errors.empty() || !na7_errors.empty()) {
        has_semantic_errors = true;
    }
}

void Parser::output_semantic_errors()
{
    // DMT-12: Duplicate polynomial declarations
    std::vector<int> all_duplicate_lines;
    for (auto& pair : duplicate_lines) {
        if (pair.second.size() > 1) {
            // Add all duplicate lines (excluding the first occurrence)
            for (int i = 1; i < (int)pair.second.size(); i++) {
                all_duplicate_lines.push_back(pair.second[i]);
            }
        }
    }
    if (!all_duplicate_lines.empty()) {
        cout << "Semantic Error Code DMT-12:";
        sort(all_duplicate_lines.begin(), all_duplicate_lines.end());
        for (int line : all_duplicate_lines) {
            cout << " " << line;
        }
        cout << endl;
        exit(1);
    }
    
    // IM-4: Invalid monomial name
    if (!im4_errors.empty()) {
        cout << "Semantic Error Code IM-4:";
        sort(im4_errors.begin(), im4_errors.end());
        for (int line : im4_errors) {
            cout << " " << line;
        }
        cout << endl;
        exit(1);
    }
    
    // AUP-13: Attempted use of undeclared polynomial
    if (!aup13_errors.empty()) {
        cout << "Semantic Error Code AUP-13:";
        sort(aup13_errors.begin(), aup13_errors.end());
        for (int line : aup13_errors) {
            cout << " " << line;
        }
        cout << endl;
        exit(1);
    }
    
    // NA-7: Wrong number of arguments
    if (!na7_errors.empty()) {
        cout << "Semantic Error Code NA-7:";
        sort(na7_errors.begin(), na7_errors.end());
        for (int line : na7_errors) {
            cout << " " << line;
        }
        cout << endl;
        exit(1);
    }
}

bool Parser::is_valid_monomial(const std::string& name)
{
    if (!current_poly) return true;
    
    for (const std::string& param : current_poly->params) {
        if (param == name) {
            return true;
        }
    }
    return false;
}

PolyDecl* Parser::find_polynomial(const std::string& name)
{
    for (PolyDecl& poly : polynomials) {
        if (poly.name == name) {
            return &poly;
        }
    }
    return nullptr;
}

void Parser::execute_tasks()
{
    // Sort task numbers
    sort(task_numbers.begin(), task_numbers.end());
    
    for (int task : task_numbers) {
        switch (task) {
            case 1:
                // Task 1 already executed (error checking)
                break;
            case 2:
                execute_task2();
                break;
            case 3:
                execute_task3();
                break;
            case 4:
                // Task 4 implementation (later)
                break;
            case 5:
                // Task 5 implementation (later)
                break;
        }
    }
}

void Parser::execute_task3()
{
    cout << "POLY - SORTED MONOMIAL LISTS" << endl;
    for (const PolyDecl& poly : polynomials) {
        print_polynomial_with_sorted_monomials(poly);
    }
}

std::vector<int> Parser::convert_to_power_array(const std::vector<Monomial>& monomials, const std::vector<std::string>& params)
{
    std::vector<int> power_array(params.size(), 0);
    
    for (const Monomial& mono : monomials) {
        // Find the position of this variable in the parameter list
        for (int i = 0; i < (int)params.size(); i++) {
            if (params[i] == mono.var_name) {
                power_array[i] += mono.exponent;
                break;
            }
        }
    }
    
    return power_array;
}

std::string Parser::format_monomial_list(const std::vector<int>& power_array, const std::vector<std::string>& params)
{
    std::string result = "";
    
    for (int i = 0; i < (int)power_array.size(); i++) {
        if (power_array[i] > 0) {
            if (!result.empty()) {
                result += " ";  // Add space between variables
            }
            result += params[i];
            if (power_array[i] > 1) {
                result += "^" + to_string(power_array[i]);
            }
        }
    }
    
    return result.empty() ? "1" : result;
}

void Parser::print_polynomial_with_sorted_monomials(const PolyDecl& poly)
{
    cout << poly.name;
    
    // Print parameter list if not default (no spaces)
    if (poly.params.size() != 1 || poly.params[0] != "x") {
        cout << "(";
        for (int i = 0; i < (int)poly.params.size(); i++) {
            if (i > 0) cout << ",";
            cout << poly.params[i];
        }
        cout << ")";
    }
    
    cout << " = ";
    
    // Process each term
    for (int term_idx = 0; term_idx < (int)poly.body.terms.size(); term_idx++) {
        const Term& term = poly.body.terms[term_idx];
        
        // Add operator for non-first terms
        if (term_idx > 0) {
            if (term.is_positive) {
                cout << " + ";
            } else {
                cout << " - ";
            }
        } else if (!term.is_positive) {
            // Handle negative first term
            cout << "-";
        }
        
        // Convert monomials to power array and sort
        std::vector<int> power_array = convert_to_power_array(term.monomials, poly.params);
        std::string monomial_str = format_monomial_list(power_array, poly.params);
        
        // Print coefficient if not 1, or if it's a constant term
        // For negative terms, we already printed the minus sign, so use absolute value
        int display_coeff = term.is_positive ? term.coefficient : term.coefficient;
        if (display_coeff != 1 || monomial_str == "1") {
            cout << display_coeff;
        }
        
        // Print monomial list (no space between coefficient and variables)
        if (monomial_str != "1") {
            cout << monomial_str;
        }
    }
    
    cout << ";" << endl;
}

// Task 2: Helper functions for program execution
int Parser::get_variable_location(const std::string& var_name)
{
    // Check if variable already has a location
    auto it = symbol_table.find(var_name);
    if (it != symbol_table.end()) {
        return it->second;
    }
    
    // Allocate new location
    int location = next_memory_location++;
    symbol_table[var_name] = location;
    
    // Ensure memory is large enough
    if ((int)memory.size() <= location) {
        memory.resize(location + 1, 0);  // initialize to 0
    }
    
    return location;
}

PolyEvaluation* Parser::parse_poly_evaluation_return()
{
    Token name_token = lexer.peek(1);
    parse_poly_name();
    expect(LPAREN);
    
    // Create polynomial evaluation structure
    PolyEvaluation* poly_eval = new PolyEvaluation();
    poly_eval->poly_name = name_token.lexeme;
    poly_eval->line_number = name_token.line_no;
    
    // Parse and store arguments
    parse_argument_list_for_evaluation(poly_eval);
    
    expect(RPAREN);
    
    // Check AUP-13: undeclared polynomial
    PolyDecl* poly = find_polynomial(name_token.lexeme);
    if (!poly) {
        aup13_errors.push_back(name_token.line_no);
    } else {
        // Check NA-7: wrong number of arguments
        if ((int)poly_eval->arguments.size() != (int)poly->params.size()) {
            na7_errors.push_back(name_token.line_no);
        }
    }
    
    return poly_eval;
}

void Parser::parse_argument_list_for_evaluation(PolyEvaluation* poly_eval)
{
    parse_argument_for_evaluation(poly_eval);
    Token t = lexer.peek(1);
    if (t.token_type == COMMA) {
        expect(COMMA);
        parse_argument_list_for_evaluation(poly_eval);
    }
}

void Parser::parse_argument_for_evaluation(PolyEvaluation* poly_eval)
{
    Token t1 = lexer.peek(1);
    Argument arg;
    
    if (t1.token_type == ID) {
        Token t2 = lexer.peek(2);
        if (t2.token_type == LPAREN) {
            // This is a nested polynomial evaluation
            arg.type = ARG_POLY;
            arg.poly_value = parse_poly_evaluation_return();
        } else {
            // This is an ID argument
            Token id_token = expect(ID);
            arg.type = ARG_ID;
            arg.id_value = id_token.lexeme;
        }
    } else if (t1.token_type == NUM) {
        // This is a NUM argument
        Token num_token = expect(NUM);
        arg.type = ARG_NUM;
        arg.num_value = stoi(num_token.lexeme);
    } else {
        syntax_error();
    }
    
    poly_eval->arguments.push_back(arg);
}

void Parser::execute_task2()
{
    // Initialize memory to sufficient size
    if (memory.size() < (size_t)next_memory_location) {
        memory.resize(next_memory_location, 0);
    }
    
    // Execute program statements in order
    for (const Statement& stmt : program_statements) {
        switch (stmt.type) {
            case INPUT_STMT:
                if (next_input_index < (int)input_values.size()) {
                    memory[stmt.var_location] = input_values[next_input_index++];
                }
                break;
                
            case OUTPUT_STMT:
                if (stmt.poly_eval) {
                    // OUTPUT with polynomial evaluation
                    int value = evaluate_polynomial(stmt.poly_eval);
                    cout << value << endl;
                } else {
                    // OUTPUT with simple variable (legacy)
                    cout << memory[stmt.var_location] << endl;
                }
                break;
                
            case ASSIGN_STMT:
                if (stmt.poly_eval) {
                    int value = evaluate_polynomial(stmt.poly_eval);
                    memory[stmt.var_location] = value;
                }
                break;
        }
    }
}

int Parser::evaluate_polynomial(const PolyEvaluation* poly_eval)
{
    // Find the polynomial declaration
    PolyDecl* poly = find_polynomial(poly_eval->poly_name);
    if (!poly) {
        return 0; // Error case, but should have been caught in semantic checking
    }
    
    // Evaluate arguments to get their values
    std::vector<int> arg_values;
    
    for (const Argument& arg : poly_eval->arguments) {
        switch (arg.type) {
            case ARG_NUM:
                arg_values.push_back(arg.num_value);
                break;
            case ARG_ID:
                {
                    auto it = symbol_table.find(arg.id_value);
                    if (it != symbol_table.end()) {
                        arg_values.push_back(memory[it->second]);
                    } else {
                        arg_values.push_back(0); // uninitialized variable
                    }
                }
                break;
            case ARG_POLY:
                {
                    int nested_value = evaluate_polynomial(arg.poly_value);
                    arg_values.push_back(nested_value);
                }
                break;
        }
    }
    
    return evaluate_polynomial_at_values(poly, arg_values);
}

int Parser::evaluate_polynomial_at_values(const PolyDecl* poly, const std::vector<int>& arg_values)
{
    int result = 0;
    
    for (const Term& term : poly->body.terms) {
        int term_value = term.coefficient;
        
        // Calculate monomial values
        for (const Monomial& mono : term.monomials) {
            // Find which parameter this monomial corresponds to
            for (int i = 0; i < (int)poly->params.size(); i++) {
                if (poly->params[i] == mono.var_name) {
                    if (i < (int)arg_values.size()) {
                        term_value *= power(arg_values[i], mono.exponent);
                    }
                    break;
                }
            }
        }
        
        // Apply the term's sign
        if (term.is_positive) {
            result += term_value;
        } else {
            result -= term_value;
        }
    }
    
    return result;
}

int Parser::power(int base, int exponent)
{
    if (exponent == 0) return 1;
    if (exponent == 1) return base;
    
    int result = 1;
    for (int i = 0; i < exponent; i++) {
        result *= base;
    }
    return result;
}

int main()
{
    Parser parser;
    parser.parse_program();
    return 0;
}