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
    has_semantic_errors = false;
    
    // Initialize task execution variables
    requested_tasks.clear();
    rich_polynomials.clear();
    current_rich_poly = nullptr;
    
    // Initialize Task 2 variables
    program.clear();
    symbol_table.clear();
    memory.clear();
    inputs.clear();
    next_input = 0;
    next_location = 0;
    
    parse_tasks_section();
    parse_poly_section();
    parse_execute_section();
    parse_inputs_section();
    expect(END_OF_FILE);
    
    // Check for semantic errors and output if found
    check_semantic_errors();
    if (has_semantic_errors) {
        output_semantic_errors();
        return; // Exit if there are semantic errors
    }
    
    // If no semantic errors, execute requested tasks
    execute_tasks();
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
    int task_num = std::stoi(num_token.lexeme);
    requested_tasks.insert(task_num); // Store task number (duplicates ignored by set)
    
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
    parse_poly_body();
    expect(SEMICOLON);
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
    
    // Also create rich polynomial representation
    RichPolyDecl rich_poly;
    rich_poly.name = poly.name;
    rich_poly.params = poly.params;
    rich_poly.line_number = poly.line_number;
    rich_poly.has_explicit_params = (t.token_type == LPAREN);  // was there an explicit param list?
    rich_polynomials.push_back(rich_poly);
    current_rich_poly = &rich_polynomials.back();
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
    if (current_rich_poly) {
        current_rich_poly->body.clear();
        parse_rich_term_list(current_rich_poly->body);
    } else {
        parse_term_list(); // For semantic checking only
    }
}

// term_list → term
// term_list → term add_operator term_list
void Parser::parse_term_list()
{
    parse_term();
    Token t = lexer.peek(1);
    if (t.token_type == PLUS || t.token_type == MINUS) {
        parse_add_operator();
        parse_term_list();
    }
}

// term → monomial_list
// term → coefficient monomial_list
// term → coefficient
// term → parenthesized_list
void Parser::parse_term()
{
    Token t1 = lexer.peek(1);
    if (t1.token_type == ID) {
        parse_monomial_list();
    } else if (t1.token_type == NUM) {
        parse_coefficient();
        Token t2 = lexer.peek(1);
        if (t2.token_type == ID) {
            parse_monomial_list();
        }
        // else just coefficient case
    } else if (t1.token_type == LPAREN) {
        parse_parenthesized_list();
    } else {
        syntax_error();
    }
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
    
    Token t = lexer.peek(1);
    if (t.token_type == POWER) {
        parse_exponent();
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
    
    // Create statement for Task 2
    Statement stmt;
    stmt.type = STMT_INPUT;
    stmt.var_index = get_or_create_variable(id_token.lexeme);
    program.push_back(stmt);
}

// output_statement → OUTPUT ID SEMICOLON
void Parser::parse_output_statement()
{
    expect(OUTPUT);
    Token id_token = expect(ID);
    expect(SEMICOLON);
    
    // Create statement for Task 2
    Statement stmt;
    stmt.type = STMT_OUTPUT;
    stmt.var_index = get_or_create_variable(id_token.lexeme);
    program.push_back(stmt);
}

// assign_statement → ID EQUAL poly_evaluation SEMICOLON
void Parser::parse_assign_statement()
{
    Token id_token = expect(ID);
    expect(EQUAL);
    
    // Parse polynomial evaluation and build representation
    PolyEval* poly_eval = parse_poly_evaluation_return();
    expect(SEMICOLON);
    
    // Create statement for Task 2
    Statement stmt;
    stmt.type = STMT_ASSIGN;
    stmt.lhs_index = get_or_create_variable(id_token.lexeme);
    stmt.rhs_eval = poly_eval;
    program.push_back(stmt);
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
    parse_inputs_num_list();
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

// Task execution functions
void Parser::execute_tasks()
{
    // Execute tasks in order: 2, 3, 4, 5 (Task 1 is always executed)
    if (requested_tasks.count(2)) {
        execute_task_2();
    }
    
    if (requested_tasks.count(3)) {
        execute_task_3();
    }
    
    if (requested_tasks.count(4)) {
        execute_task_4();
    }
    
    if (requested_tasks.count(5)) {
        execute_task_5();
    }
}

void Parser::execute_task_3()
{
    // For now, just output placeholder
    if (!rich_polynomials.empty()) {
        cout << "POLY - SORTED MONOMIAL LISTS" << endl;
        for (const auto& poly : rich_polynomials) {
            cout << "\t" << format_poly_decl(poly) << ";" << endl;
        }
    }
}

// Helper functions for Task 3 - temporary implementations
std::string Parser::format_monomial_list(const std::vector<int>& powers, const std::vector<std::string>& params)
{
    std::string result;
    bool first = true;
    
    for (int i = 0; i < (int)powers.size() && i < (int)params.size(); i++) {
        if (powers[i] > 0) {
            if (!first) result += " ";
            first = false;
            
            result += params[i];
            if (powers[i] > 1) {
                result += "^" + std::to_string(powers[i]);
            }
        }
    }
    
    return result.empty() ? "1" : result;
}

std::string Parser::format_term(const TermNode& term, const std::vector<std::string>& params, bool is_first)
{
    std::string result;
    
    // Add operator (except for first positive term)
    if (!is_first) {
        result += (term.op == OP_PLUS) ? " + " : " - ";
    } else if (term.op == OP_MINUS) {
        result += "- ";
    }
    
    if (term.kind == MLIST) {
        // Add coefficient if not 1 or if it's a constant term
        bool all_zero = true;
        for (int power : term.monomial_list) {
            if (power != 0) {
                all_zero = false;
                break;
            }
        }
        
        if (term.coefficient != 1 || all_zero) {
            result += std::to_string(abs(term.coefficient));
            if (!all_zero) result += " ";
        }
        
        if (!all_zero) {
            result += format_monomial_list(term.monomial_list, params);
        }
    } else {
        // PARENLIST - format parenthesized lists
        result += format_parenthesized_list(term.parenthesized_list, params);
    }
    
    return result;
}

std::string Parser::format_poly_decl(const RichPolyDecl& poly)
{
    std::string result = poly.name;
    
    // Show parameters if: 
    // 1. More than one parameter, OR
    // 2. Explicitly specified parameter list (even if it's just "x")
    if (poly.params.size() > 1 || poly.has_explicit_params) {
        result += "(";
        for (int i = 0; i < (int)poly.params.size(); i++) {
            if (i > 0) result += ",";
            result += poly.params[i];
        }
        result += ")";
    }
    
    result += " = ";
    
    // Format terms
    if (!poly.body.empty()) {
        result += format_term(poly.body[0], poly.params, true);
        for (int i = 1; i < (int)poly.body.size(); i++) {
            result += format_term(poly.body[i], poly.params, false);
        }
    } else {
        result += "0"; // Empty polynomial
    }
    
    return result;
}

std::string Parser::format_parenthesized_list(const std::vector<std::vector<TermNode>>& paren_list, const std::vector<std::string>& params)
{
    std::string result;
    
    // For Task 3, we don't expand parenthesized lists, just format them properly
    for (size_t i = 0; i < paren_list.size(); i++) {
        result += "(";
        
        const std::vector<TermNode>& term_list = paren_list[i];
        if (!term_list.empty()) {
            result += format_term(term_list[0], params, true);
            for (size_t j = 1; j < term_list.size(); j++) {
                result += format_term(term_list[j], params, false);
            }
        }
        
        result += ")";
    }
    
    return result;
}

// Rich parsing functions - actual implementations
void Parser::parse_rich_poly_body(std::vector<TermNode>& terms)
{
    parse_rich_term_list(terms);
}

void Parser::parse_rich_term_list(std::vector<TermNode>& terms)
{
    // term_list → term
    // term_list → term add_operator term_list
    
    TermNode term;
    term.op = OP_PLUS; // First term is always positive
    parse_rich_term(term);
    terms.push_back(term);
    
    Token t = lexer.peek(1);
    if (t.token_type == PLUS || t.token_type == MINUS) {
        parse_add_operator(); // Consume the operator
        
        // Set up the next term with the operator
        std::vector<TermNode> rest_terms;
        parse_rich_term_list(rest_terms);
        
        // Set the operator for the first term in rest_terms
        if (!rest_terms.empty()) {
            rest_terms[0].op = (t.token_type == PLUS) ? OP_PLUS : OP_MINUS;
            terms.insert(terms.end(), rest_terms.begin(), rest_terms.end());
        }
    }
}

void Parser::parse_rich_term(TermNode& term)
{
    // term → monomial_list
    // term → coefficient monomial_list  
    // term → coefficient
    // term → parenthesized_list
    
    term.kind = MLIST;
    term.coefficient = 1;
    term.monomial_list.clear();
    term.monomial_list.resize(current_rich_poly->params.size(), 0);
    
    Token t1 = lexer.peek(1);
    if (t1.token_type == ID) {
        // monomial_list
        parse_rich_monomial_list(term.monomial_list);
    } else if (t1.token_type == NUM) {
        // coefficient [monomial_list]
        term.coefficient = parse_rich_coefficient();
        Token t2 = lexer.peek(1);
        if (t2.token_type == ID) {
            parse_rich_monomial_list(term.monomial_list);
        }
        // else just coefficient case (monomial_list stays all zeros)
    } else if (t1.token_type == LPAREN) {
        // parenthesized_list
        term.kind = PARENLIST;
        parse_rich_parenthesized_list(term.parenthesized_list);
    } else {
        syntax_error();
    }
}

void Parser::parse_rich_monomial_list(std::vector<int>& powers)
{
    // monomial_list → monomial
    // monomial_list → monomial monomial_list
    
    // Parse one monomial
    Token id_token = expect(ID);
    
    // Check if this monomial name is valid (IM-4 check)
    if (current_poly && !is_valid_monomial(id_token.lexeme)) {
        im4_errors.push_back(id_token.line_no);
    }
    
    // Find the parameter index
    std::string var_name = id_token.lexeme;
    int param_index = -1;
    for (int i = 0; i < (int)current_rich_poly->params.size(); i++) {
        if (current_rich_poly->params[i] == var_name) {
            param_index = i;
            break;
        }
    }
    
    // Parse optional exponent
    int exponent = 1;
    Token t = lexer.peek(1);
    if (t.token_type == POWER) {
        expect(POWER);
        Token exp_token = expect(NUM);
        exponent = std::stoi(exp_token.lexeme);
    }
    
    // Add to powers array
    if (param_index >= 0) {
        powers[param_index] += exponent;
    }
    
    // Check for more monomials
    t = lexer.peek(1);
    if (t.token_type == ID) {
        parse_rich_monomial_list(powers);
    }
}

int Parser::parse_rich_coefficient()
{
    Token t = expect(NUM);
    return std::stoi(t.lexeme);
}

void Parser::parse_rich_parenthesized_list(std::vector<std::vector<TermNode>>& paren_list)
{
    // parenthesized_list → LPAREN term_list RPAREN
    // parenthesized_list → LPAREN term_list RPAREN parenthesized_list
    
    expect(LPAREN);
    std::vector<TermNode> term_list;
    parse_rich_term_list(term_list);
    expect(RPAREN);
    
    paren_list.push_back(term_list);
    
    Token t = lexer.peek(1);
    if (t.token_type == LPAREN) {
        parse_rich_parenthesized_list(paren_list);
    }
}

// Task 2 implementation functions
void Parser::parse_inputs_num_list()
{
    Token num_token = expect(NUM);
    int value = std::stoi(num_token.lexeme);
    inputs.push_back(value);
    
    Token t = lexer.peek(1);
    if (t.token_type == NUM) {
        parse_inputs_num_list();
    }
}

int Parser::get_or_create_variable(const std::string& name)
{
    if (symbol_table.find(name) == symbol_table.end()) {
        symbol_table[name] = next_location++;
        memory.resize(next_location, 0); // Initialize to 0
    }
    return symbol_table[name];
}

PolyEval* Parser::parse_poly_evaluation_return()
{
    Token name_token = lexer.peek(1);
    parse_poly_name();
    expect(LPAREN);
    
    // Create evaluation structure
    PolyEval* eval = new PolyEval();
    
    // Find polynomial index
    for (int i = 0; i < (int)rich_polynomials.size(); i++) {
        if (rich_polynomials[i].name == name_token.lexeme) {
            eval->poly_index = i;
            break;
        }
    }
    
    // Parse arguments
    parse_argument_list_return(eval->args);
    
    expect(RPAREN);
    
    // Still do semantic checks
    PolyDecl* poly = find_polynomial(name_token.lexeme);
    if (!poly) {
        aup13_errors.push_back(name_token.line_no);
    } else {
        if ((int)eval->args.size() != (int)poly->params.size()) {
            na7_errors.push_back(name_token.line_no);
        }
    }
    
    return eval;
}

void Parser::parse_argument_list_return(std::vector<PolyArgument>& args)
{
    PolyArgument arg = parse_argument_return();
    args.push_back(arg);
    
    Token t = lexer.peek(1);
    if (t.token_type == COMMA) {
        expect(COMMA);
        parse_argument_list_return(args);
    }
}

PolyArgument Parser::parse_argument_return()
{
    PolyArgument arg;
    
    Token t1 = lexer.peek(1);
    if (t1.token_type == ID) {
        Token t2 = lexer.peek(2);
        if (t2.token_type == LPAREN) {
            // Nested polynomial evaluation
            arg.kind = ARG_POLYEVAL;
            arg.poly_eval = parse_poly_evaluation_return();
        } else {
            // Variable ID
            Token id_token = expect(ID);
            arg.kind = ARG_ID;
            arg.var_index = get_or_create_variable(id_token.lexeme);
        }
    } else if (t1.token_type == NUM) {
        // Numeric constant
        Token num_token = expect(NUM);
        arg.kind = ARG_NUM;
        arg.value = std::stoi(num_token.lexeme);
    } else {
        syntax_error();
    }
    
    return arg;
}

void Parser::execute_task_2()
{
    // Execute the program by going through the statement list
    for (const Statement& stmt : program) {
        switch (stmt.type) {
            case STMT_INPUT:
                if (stmt.var_index >= 0 && stmt.var_index < (int)memory.size()) {
                    if (next_input < (int)inputs.size()) {
                        memory[stmt.var_index] = inputs[next_input];
                    }
                    // Always increment next_input, even if no more inputs available
                    next_input++;
                }
                break;
                
            case STMT_OUTPUT:
                if (stmt.var_index >= 0 && stmt.var_index < (int)memory.size()) {
                    std::cout << memory[stmt.var_index] << std::endl;
                } else {
                    std::cout << 0 << std::endl; // Default for invalid index
                }
                break;
                
            case STMT_ASSIGN:
                if (stmt.lhs_index >= 0 && stmt.lhs_index < (int)memory.size()) {
                    memory[stmt.lhs_index] = evaluate_polynomial(stmt.rhs_eval);
                }
                break;
        }
    }
}

int Parser::evaluate_polynomial(const PolyEval* eval)
{
    if (!eval || eval->poly_index < 0 || eval->poly_index >= (int)rich_polynomials.size()) {
        return 0; // Error case
    }
    
    const RichPolyDecl& poly = rich_polynomials[eval->poly_index];
    
    // Evaluate all arguments
    std::vector<int> arg_values;
    for (const PolyArgument& arg : eval->args) {
        arg_values.push_back(evaluate_argument(arg));
    }
    
    // Ensure we have the right number of argument values
    if ((int)arg_values.size() != (int)poly.params.size()) {
        return 0; // Argument count mismatch - should not happen after semantic checking
    }
    
    // Evaluate polynomial body (term list)
    int result = 0;
    bool first = true;
    
    for (const TermNode& term : poly.body) {
        int term_value = evaluate_term(term, arg_values);
        
        if (first) {
            result = (term.op == OP_MINUS) ? -term_value : term_value;
            first = false;
        } else {
            result += (term.op == OP_PLUS) ? term_value : -term_value;
        }
    }
    
    return result;
}

int Parser::evaluate_argument(const PolyArgument& arg)
{
    switch (arg.kind) {
        case ARG_NUM:
            return arg.value;
            
        case ARG_ID:
            if (arg.var_index >= 0 && arg.var_index < (int)memory.size()) {
                return memory[arg.var_index];
            }
            return 0;
            
        case ARG_POLYEVAL:
            return evaluate_polynomial(arg.poly_eval);
            
        default:
            return 0;
    }
}

int Parser::evaluate_term(const TermNode& term, const std::vector<int>& arg_values)
{
    if (term.kind == MLIST) {
        // Monomial list term: coefficient * product of variable powers
        int result = term.coefficient;
        
        for (int i = 0; i < (int)term.monomial_list.size() && i < (int)arg_values.size(); i++) {
            if (term.monomial_list[i] > 0) {
                result *= int_power(arg_values[i], term.monomial_list[i]);
            }
        }
        
        return result;
    } else {
        // PARENLIST - evaluate each parenthesized term list and multiply them
        int result = 1;
        
        for (const std::vector<TermNode>& term_list : term.parenthesized_list) {
            // Evaluate this term list
            int term_list_value = 0;
            bool first = true;
            
            for (const TermNode& inner_term : term_list) {
                int inner_value = evaluate_term(inner_term, arg_values);
                
                if (first) {
                    term_list_value = (inner_term.op == OP_MINUS) ? -inner_value : inner_value;
                    first = false;
                } else {
                    term_list_value += (inner_term.op == OP_PLUS) ? inner_value : -inner_value;
                }
            }
            
            result *= term_list_value;
        }
        
        return result;
    }
}

int Parser::int_power(int base, int exp)
{
    int result = 1;
    for (int i = 0; i < exp; i++) {
        result *= base;
    }
    return result;
}

void Parser::execute_task_4()
{
    if (!rich_polynomials.empty()) {
        cout << "POLY - COMBINED MONOMIAL LISTS" << endl;
        
        // Create a copy of polynomials and combine identical monomial lists
        std::vector<RichPolyDecl> combined_polynomials;
        
        for (const auto& poly : rich_polynomials) {
            RichPolyDecl combined_poly = poly;
            combined_poly.body = combine_identical_monomials(poly.body, poly.params);
            combined_polynomials.push_back(combined_poly);
        }
        
        for (const auto& poly : combined_polynomials) {
            cout << "\t" << format_poly_decl(poly) << ";" << endl;
        }
    }
}

std::vector<TermNode> Parser::combine_identical_monomials(const std::vector<TermNode>& terms, const std::vector<std::string>& params)
{
    std::vector<TermNode> result;
    
    for (const auto& term : terms) {
        if (term.kind == MLIST) {
            // Find if we already have a term with identical monomial list
            bool found = false;
            for (auto& existing : result) {
                if (existing.kind == MLIST && 
                    monomials_are_identical(existing.monomial_list, term.monomial_list)) {
                    
                    // Combine coefficients, considering the sign
                    int term_coeff = term.coefficient;
                    if (term.op == OP_MINUS) {
                        term_coeff = -term_coeff;
                    }
                    
                    int existing_coeff = existing.coefficient;
                    if (existing.op == OP_MINUS) {
                        existing_coeff = -existing_coeff;
                    }
                    
                    int combined_coeff = existing_coeff + term_coeff;
                    
                    // Update the existing term with combined coefficient
                    if (combined_coeff >= 0) {
                        existing.coefficient = combined_coeff;
                        existing.op = OP_PLUS;
                    } else {
                        existing.coefficient = -combined_coeff;
                        existing.op = OP_MINUS;
                    }
                    
                    found = true;
                    break;
                }
            }
            
            if (!found) {
                // Add this term as a new entry
                result.push_back(term);
            }
        } else {
            // PARENLIST - recursively combine terms within each parenthesized list
            TermNode combined_term = term;
            for (auto& paren_list : combined_term.parenthesized_list) {
                paren_list = combine_identical_monomials(paren_list, params);
            }
            result.push_back(combined_term);
        }
    }
    
    // Remove terms with coefficient 0, but keep at least one zero term if result would be empty
    auto new_end = std::remove_if(result.begin(), result.end(), 
        [](const TermNode& term) { 
            return term.kind == MLIST && term.coefficient == 0; 
        });
    
    // If removing zero terms would make result empty, add a single zero constant term
    if (new_end == result.begin()) {
        TermNode zero_term;
        zero_term.kind = MLIST;
        zero_term.op = OP_PLUS;
        zero_term.coefficient = 0;
        zero_term.monomial_list.resize(params.size(), 0); // All zeros = constant term
        result.clear();
        result.push_back(zero_term);
    } else {
        result.erase(new_end, result.end());
    }
    
    return result;
}

bool Parser::monomials_are_identical(const std::vector<int>& mono1, const std::vector<int>& mono2)
{
    if (mono1.size() != mono2.size()) {
        return false;
    }
    
    for (size_t i = 0; i < mono1.size(); i++) {
        if (mono1[i] != mono2[i]) {
            return false;
        }
    }
    
    return true;
}

void Parser::execute_task_5()
{
    if (!rich_polynomials.empty()) {
        cout << "POLY - EXPANDED" << endl;
        
        // Create expanded polynomials
        std::vector<RichPolyDecl> expanded_polynomials;
        
        for (const auto& poly : rich_polynomials) {
            RichPolyDecl expanded_poly = poly;
            expanded_poly.body = expand_polynomial(poly.body, poly.params);
            expanded_polynomials.push_back(expanded_poly);
        }
        
        for (const auto& poly : expanded_polynomials) {
            cout << "\t" << format_poly_decl(poly) << ";" << endl;
        }
    }
}

std::vector<TermNode> Parser::expand_polynomial(const std::vector<TermNode>& terms, const std::vector<std::string>& params)
{
    std::vector<TermNode> expanded_terms;
    
    for (const auto& term : terms) {
        if (term.kind == MLIST) {
            // Regular monomial list - just add it
            expanded_terms.push_back(term);
        } else if (term.kind == PARENLIST) {
            // PARENLIST - expand it
            std::vector<TermNode> expanded_paren = expand_parenthesized_term(term, params);
            expanded_terms.insert(expanded_terms.end(), expanded_paren.begin(), expanded_paren.end());
        }
        // Skip any unknown term kinds
    }
    
    // Combine like terms after expansion
    std::vector<TermNode> combined = combine_identical_monomials(expanded_terms, params);
    
    // Sort the terms to ensure consistent ordering
    sort_terms(combined, params);
    
    return combined;
}

std::vector<TermNode> Parser::expand_parenthesized_term(const TermNode& paren_term, const std::vector<std::string>& params)
{
    std::vector<TermNode> result;
    
    if (paren_term.parenthesized_list.empty()) {
        return result;
    }
    
    // First, recursively expand each parenthesized list
    std::vector<std::vector<TermNode>> expanded_lists;
    for (const auto& paren_list : paren_term.parenthesized_list) {
        std::vector<TermNode> expanded_list;
        for (const auto& term : paren_list) {
            if (term.kind == PARENLIST) {
                // Recursively expand nested parentheses
                std::vector<TermNode> nested_expansion = expand_parenthesized_term(term, params);
                expanded_list.insert(expanded_list.end(), nested_expansion.begin(), nested_expansion.end());
            } else {
                expanded_list.push_back(term);
            }
        }
        expanded_lists.push_back(expanded_list);
    }
    
    // Now multiply all the expanded lists together
    std::vector<TermNode> current_expansion = expanded_lists[0];
    for (size_t i = 1; i < expanded_lists.size(); i++) {
        current_expansion = multiply_term_lists(current_expansion, expanded_lists[i], params);
    }
    
    // Apply the operator and coefficient of the parenthesized term
    for (auto& term : current_expansion) {
        if (term.kind == MLIST) {
            // Handle coefficient multiplication
            int original_coeff = term.coefficient;
            if (term.op == OP_MINUS) {
                original_coeff = -original_coeff;
            }
            
            int final_coeff = original_coeff * paren_term.coefficient;
            if (paren_term.op == OP_MINUS) {
                final_coeff = -final_coeff;
            }
            
            // Set the final sign and coefficient
            if (final_coeff >= 0) {
                term.coefficient = final_coeff;
                term.op = OP_PLUS;
            } else {
                term.coefficient = -final_coeff;
                term.op = OP_MINUS;
            }
        }
    }
    
    return current_expansion;
}

std::vector<TermNode> Parser::multiply_term_lists(const std::vector<TermNode>& list1, const std::vector<TermNode>& list2, const std::vector<std::string>& params)
{
    std::vector<TermNode> result;
    
    for (const auto& term1 : list1) {
        for (const auto& term2 : list2) {
            if (term1.kind == MLIST && term2.kind == MLIST) {
                TermNode product;
                product.kind = MLIST;
                product.op = OP_PLUS; // We'll handle signs later
                
                // Multiply coefficients with proper sign handling
                int coeff1 = term1.coefficient;
                if (term1.op == OP_MINUS) coeff1 = -coeff1;
                
                int coeff2 = term2.coefficient;
                if (term2.op == OP_MINUS) coeff2 = -coeff2;
                
                int product_coeff = coeff1 * coeff2;
                
                // Handle the case where product is zero
                if (product_coeff == 0) {
                    product.coefficient = 0;
                    product.op = OP_PLUS;
                } else if (product_coeff > 0) {
                    product.coefficient = product_coeff;
                    product.op = OP_PLUS;
                } else {
                    product.coefficient = -product_coeff;
                    product.op = OP_MINUS;
                }
                
                // Add powers for each parameter
                product.monomial_list.resize(params.size(), 0);
                for (size_t i = 0; i < params.size() && i < term1.monomial_list.size(); i++) {
                    product.monomial_list[i] += term1.monomial_list[i];
                }
                for (size_t i = 0; i < params.size() && i < term2.monomial_list.size(); i++) {
                    product.monomial_list[i] += term2.monomial_list[i];
                }
                
                result.push_back(product);
            }
        }
    }
    
    return result;
}

void Parser::sort_terms(std::vector<TermNode>& terms, const std::vector<std::string>& params)
{
    // Sort terms by their monomial signature for consistent ordering
    std::sort(terms.begin(), terms.end(), [this, &params](const TermNode& a, const TermNode& b) {
        return term_less_than(a, b, params);
    });
}

bool Parser::term_less_than(const TermNode& a, const TermNode& b, const std::vector<std::string>& params)
{
    if (a.kind != MLIST || b.kind != MLIST) {
        return false; // Don't sort non-MLIST terms
    }
    
    // Compare by total degree first (sum of all powers)
    int degree_a = 0, degree_b = 0;
    for (int power : a.monomial_list) degree_a += power;
    for (int power : b.monomial_list) degree_b += power;
    
    if (degree_a != degree_b) {
        return degree_a > degree_b; // Higher degree terms first
    }
    
    // If same total degree, compare lexicographically by individual powers
    size_t min_size = std::min(a.monomial_list.size(), b.monomial_list.size());
    for (size_t i = 0; i < min_size; i++) {
        if (a.monomial_list[i] != b.monomial_list[i]) {
            return a.monomial_list[i] > b.monomial_list[i]; // Higher power first
        }
    }
    
    return a.monomial_list.size() < b.monomial_list.size();
}

int main()
{
    Parser parser;
    parser.parse_program();
    return 0;
}