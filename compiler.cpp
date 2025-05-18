#include "compiler.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>
#include <cctype>
#include <cmath>
#include <iomanip>
#include <regex>

// Define math constants if not available
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#ifndef M_E
#define M_E 2.71828182845904523536
#endif

Compiler::Compiler() {
    // Initialize operator arities (number of operands)
    operatorArities["+"] = 2;
    operatorArities["-"] = 2;
    operatorArities["*"] = 2;
    operatorArities["/"] = 2;
    operatorArities["^"] = 2;
    operatorArities["%"] = 2;
    operatorArities["!"] = 1;
    operatorArities["abs"] = 1;
    operatorArities["sin"] = 1;
    operatorArities["cos"] = 1;
    operatorArities["tan"] = 1;
    operatorArities["sqrt"] = 1;
    operatorArities["swap"] = 2;
    operatorArities["dup"] = 1;
    
    // Initialize constants
    constants["pi"] = M_PI;
    constants["e"] = M_E;
}

void Compiler::compile(const std::string& expression, const std::string& outputFile) {
    std::vector<Token> tokens = tokenize(expression);
    
    std::ofstream outFile(outputFile);
    if (!outFile) {
        throw std::runtime_error("Failed to open output file for writing");
    }
    
    generateAssembly(tokens, outFile);
}

std::string Compiler::compileToString(const std::string& expression) {
    std::vector<Token> tokens = tokenize(expression);
    std::ostringstream oss;
    
    generateAssembly(tokens, oss);
    
    return oss.str();
}

std::vector<Token> Compiler::tokenize(const std::string& expression) {
    std::vector<Token> tokens;
    std::istringstream iss(expression);
    std::string token;
    
    while (iss >> token) {
        // Check if it's a number (including scientific notation)
        bool isNumber = false;
        size_t pos = 0;
        try {
            // stod supports scientific notation like 1.23e5
            std::stod(token, &pos);
            isNumber = (pos == token.size());
            
            // Also validate scientific notation format explicitly
            if (!isNumber) {
                std::regex scientificNotationRegex("^[-+]?[0-9]*\\.?[0-9]+([eE][-+]?[0-9]+)?$");
                isNumber = std::regex_match(token, scientificNotationRegex);
            }
        } catch (...) {
            isNumber = false;
        }
        
        if (isNumber) {
            tokens.push_back(Token(Token::NUMBER, token));
        } 
        // Check if it's a constant
        else if (constants.find(token) != constants.end()) {
            tokens.push_back(Token(Token::CONSTANT, token));
        }
        // Check if it's a stack operation
        else if (token == "swap" || token == "dup") {
            tokens.push_back(Token(Token::STACK_OP, token));
        }
        // Check if it's a function
        else if (token == "abs" || token == "sin" || token == "cos" || 
                 token == "tan" || token == "sqrt") {
            tokens.push_back(Token(Token::FUNCTION, token));
        }
        // Must be an operator
        else if (token == "+" || token == "-" || token == "*" || token == "/" ||
                 token == "^" || token == "%" || token == "!") {
            tokens.push_back(Token(Token::OPERATOR, token));
        }
        else {
            throw std::runtime_error("Unknown token: " + token);
        }
    }
    
    return tokens;
}

void Compiler::generateAssembly(const std::vector<Token>& tokens, std::ostream& out) {
    // Generate assembly header
    out << "; Math compiler output\n";
    out << "; Generated assembly for x86-64\n\n";
    
    out << "section .data\n";
    // Define constants used in the program
    out << "    format db \"%lf\", 10, 0  ; Format for printf\n";
    out << "    div_zero_msg db \"Error: Division by zero\", 10, 0\n";
    out << "    stack_underflow_msg db \"Error: Stack underflow\", 10, 0\n";
    
    // Define used constants
    for (auto& token : tokens) {
        if (token.type == Token::CONSTANT) {
            out << "    " << token.strValue << " dq " << std::fixed << std::setprecision(17) 
                << constants[token.strValue] << "\n";
        }
    }
    
    out << "\nsection .text\n";
    out << "    global main\n";
    out << "    extern printf\n";
    out << "    extern exit\n\n";
    
    // Implement main function
    out << "main:\n";
    out << "    ; Set up stack frame\n";
    out << "    push rbp\n";
    out << "    mov rbp, rsp\n";
    out << "    sub rsp, 16*32  ; Reserve space for 32 doubles on the stack\n\n";
    
    out << "    ; Initialize stack pointer\n";
    out << "    mov r12, 0  ; r12 = stack pointer (number of items on stack)\n\n";
    
    // Process each token
    int labelCounter = 0;
    
    for (size_t i = 0; i < tokens.size(); i++) {
        const Token& token = tokens[i];
        
        out << "    ; Process token: " << token.strValue << "\n";
        
        switch (token.type) {
            case Token::NUMBER: {
                out << "    ; Push number onto stack\n";
                out << "    movsd xmm0, __float64__(" << token.numValue << ")\n";
                out << "    call push_stack\n";
                break;
            }
            
            case Token::CONSTANT: {
                out << "    ; Push constant onto stack\n";
                out << "    movsd xmm0, [rel " << token.strValue << "]\n";
                out << "    call push_stack\n";
                break;
            }
            
            case Token::OPERATOR: {
                int arity = operatorArities[token.strValue];
                
                out << "    ; Check if we have enough operands\n";
                out << "    cmp r12, " << arity << "\n";
                out << "    jl stack_underflow\n\n";
                
                if (token.strValue == "+") {
                    out << "    ; Addition\n";
                    out << "    call pop_stack  ; Get first operand into xmm0\n";
                    out << "    movsd xmm1, xmm0\n";
                    out << "    call pop_stack  ; Get second operand into xmm0\n";
                    out << "    addsd xmm0, xmm1\n";
                    out << "    call push_stack\n";
                }
                else if (token.strValue == "-") {
                    out << "    ; Subtraction\n";
                    out << "    call pop_stack  ; Get first operand into xmm0\n";
                    out << "    movsd xmm1, xmm0\n";
                    out << "    call pop_stack  ; Get second operand into xmm0\n";
                    out << "    subsd xmm0, xmm1\n";
                    out << "    call push_stack\n";
                }
                else if (token.strValue == "*") {
                    out << "    ; Multiplication\n";
                    out << "    call pop_stack  ; Get first operand into xmm0\n";
                    out << "    movsd xmm1, xmm0\n";
                    out << "    call pop_stack  ; Get second operand into xmm0\n";
                    out << "    mulsd xmm0, xmm1\n";
                    out << "    call push_stack\n";
                }
                else if (token.strValue == "/") {
                    out << "    ; Division\n";
                    out << "    call pop_stack  ; Get divisor into xmm0\n";
                    out << "    ; Check if divisor is zero\n";
                    out << "    xorpd xmm1, xmm1\n";
                    out << "    ucomisd xmm0, xmm1\n";
                    out << "    je division_by_zero\n";
                    out << "    movsd xmm1, xmm0\n";
                    out << "    call pop_stack  ; Get dividend into xmm0\n";
                    out << "    divsd xmm0, xmm1\n";
                    out << "    call push_stack\n";
                }
                else if (token.strValue == "^") {
                    // We'll use the C library pow function
                    labelCounter++;
                    out << "    ; Power (x^y)\n";
                    out << "    call pop_stack  ; Get exponent into xmm0\n";
                    out << "    movsd xmm1, xmm0\n";
                    out << "    call pop_stack  ; Get base into xmm0\n";
                    
                    // We use a simple approach for integer powers
                    out << "    ; Check if exponent is an integer\n";
                    out << "    cvttsd2si rax, xmm1  ; Convert to integer truncating\n";
                    out << "    cvtsi2sd xmm2, rax   ; Convert back to double\n";
                    out << "    ucomisd xmm1, xmm2   ; Compare original and converted value\n";
                    out << "    jne power_general_" << labelCounter << "\n";
                    
                    // Integer power implementation
                    out << "    ; Integer power implementation\n";
                    out << "    movsd xmm2, __float64__(1.0)  ; Result accumulator\n";
                    out << "    test rax, rax\n";
                    out << "    jz power_done_" << labelCounter << "  ; x^0 = 1\n";
                    out << "    js power_general_" << labelCounter << "  ; Negative exponent needs general case\n";
                    
                    out << "power_loop_" << labelCounter << ":\n";
                    out << "    test rax, 1\n";
                    out << "    jz power_skip_" << labelCounter << "\n";
                    out << "    mulsd xmm2, xmm0  ; Multiply result by x\n";
                    out << "power_skip_" << labelCounter << ":\n";
                    out << "    mulsd xmm0, xmm0  ; Square x\n";
                    out << "    shr rax, 1        ; Divide exponent by 2\n";
                    out << "    jnz power_loop_" << labelCounter << "\n";
                    out << "    movsd xmm0, xmm2\n";
                    out << "    jmp power_done_" << labelCounter << "\n";
                    
                    // General power using exp and log
                    out << "power_general_" << labelCounter << ":\n";
                    out << "    ; x^y = exp(y * ln(x))\n";
                    out << "    ; Check if x > 0 for log\n";
                    out << "    xorpd xmm2, xmm2\n";
                    out << "    ucomisd xmm0, xmm2\n";
                    out << "    jbe power_error_" << labelCounter << "  ; If x <= 0, can't take log\n";
                    
                    out << "    sub rsp, 8  ; Align stack\n";
                    out << "    movq [rsp], xmm0  ; Save x\n";
                    out << "    movq [rsp-8], xmm1  ; Save y\n";
                    
                    out << "    call log  ; Get ln(x) in xmm0\n";
                    out << "    movsd xmm1, [rsp-8]  ; Restore y to xmm1\n";
                    out << "    mulsd xmm0, xmm1  ; y * ln(x)\n";
                    out << "    call exp  ; exp(y * ln(x))\n";
                    
                    out << "    add rsp, 8  ; Restore stack\n";
                    out << "    jmp power_done_" << labelCounter << "\n";
                    
                    out << "power_error_" << labelCounter << ":\n";
                    out << "    ; Handle error case (probably not ideal but simple)\n";
                    out << "    movsd xmm0, __float64__(0.0)\n";
                    
                    out << "power_done_" << labelCounter << ":\n";
                    out << "    call push_stack\n";
                }
                else if (token.strValue == "%") {
                    out << "    ; Modulus\n";
                    out << "    call pop_stack  ; Get second operand into xmm0\n";
                    out << "    ; Check if divisor is zero\n";
                    out << "    xorpd xmm1, xmm1\n";
                    out << "    ucomisd xmm0, xmm1\n";
                    out << "    je division_by_zero\n";
                    out << "    movsd xmm1, xmm0\n";
                    out << "    call pop_stack  ; Get first operand into xmm0\n";
                    
                    // Implementation of floating-point modulus
                    labelCounter++;
                    out << "    ; Floating-point modulus: x % y = x - y * floor(x/y)\n";
                    out << "    movsd xmm2, xmm0  ; Save x\n";
                    out << "    divsd xmm0, xmm1  ; x / y\n";
                    
                    // Compute floor(x/y)
                    out << "    sub rsp, 8  ; Align stack\n";
                    out << "    movq [rsp], xmm1  ; Save y\n";
                    out << "    call floor  ; Get floor(x/y)\n";
                    out << "    movsd xmm1, [rsp]  ; Restore y from stack\n";
                    out << "    add rsp, 8  ; Restore stack\n";
                    
                    out << "    mulsd xmm0, xmm1  ; y * floor(x/y)\n";
                    out << "    movsd xmm1, xmm2  ; Restore x\n";
                    out << "    subsd xmm1, xmm0  ; x - y * floor(x/y)\n";
                    out << "    movsd xmm0, xmm1\n";
                    out << "    call push_stack\n";
                }
                else if (token.strValue == "!") {
                    out << "    ; Factorial\n";
                    out << "    call pop_stack  ; Get operand into xmm0\n";
                    
                    // Convert to integer and compute factorial
                    labelCounter++;
                    out << "    ; Convert to integer\n";
                    out << "    cvttsd2si rax, xmm0\n";
                    
                    // Check if conversion was accurate - factorial only defined for non-negative integers
                    out << "    cvtsi2sd xmm1, rax\n";
                    out << "    ucomisd xmm0, xmm1\n";
                    out << "    jne factorial_error_" << labelCounter << "\n";
                    
                    out << "    ; Check if n < 0\n";
                    out << "    test rax, rax\n";
                    out << "    js factorial_error_" << labelCounter << "\n";
                    
                    // Check for potential overflow - factorial grows very quickly
                    out << "    ; Check for potential overflow (n > 20 will overflow 64-bit)\n";
                    out << "    cmp rax, 20\n";
                    out << "    jg factorial_overflow_" << labelCounter << "\n";
                    
                    out << "    ; Compute factorial\n";
                    out << "    mov rcx, 1  ; Result\n";
                    out << "    test rax, rax\n";
                    out << "    jz factorial_done_" << labelCounter << "  ; 0! = 1\n";
                    
                    out << "factorial_loop_" << labelCounter << ":\n";
                    out << "    imul rcx, rax  ; result *= n\n";
                    // Check for multiplication overflow
                    out << "    jo factorial_overflow_" << labelCounter << "  ; Jump if overflow occurred\n";
                    out << "    dec rax        ; n--\n";
                    out << "    jnz factorial_loop_" << labelCounter << "\n";
                    
                    out << "factorial_done_" << labelCounter << ":\n";
                    out << "    cvtsi2sd xmm0, rcx  ; Convert result to double\n";
                    out << "    jmp factorial_end_" << labelCounter << "\n";
                    
                    out << "factorial_overflow_" << labelCounter << ":\n";
                    out << "    ; Handle overflow - calculate using floating-point for large values\n";
                    out << "    ; Simple implementation - convert back to double to avoid overflow\n";
                    out << "    cvtsi2sd xmm0, rax  ; Convert n to double\n";
                    out << "    movsd xmm2, __float64__(1.0)  ; Result\n";
                    
                    out << "factorial_fp_loop_" << labelCounter << ":\n";
                    out << "    mulsd xmm2, xmm0  ; result *= n\n";
                    out << "    subsd xmm0, __float64__(1.0)  ; n--\n";
                    out << "    movsd xmm3, __float64__(0.0)  ; For comparison\n";
                    out << "    ucomisd xmm0, xmm3\n";
                    out << "    ja factorial_fp_loop_" << labelCounter << "\n";
                    
                    out << "    movsd xmm0, xmm2  ; Move result to xmm0\n";
                    out << "    jmp factorial_end_" << labelCounter << "\n";
                    
                    out << "factorial_error_" << labelCounter << ":\n";
                    out << "    ; Factorial not defined for this input (not a non-negative integer)\n";
                    out << "    movsd xmm0, __float64__(0.0)  ; Return 0 as error value\n";
                    
                    out << "factorial_end_" << labelCounter << ":\n";
                    out << "    call push_stack\n";
                }
                break;
            }
            
            case Token::FUNCTION: {
                int arity = operatorArities[token.strValue];
                
                out << "    ; Check if we have enough operands\n";
                out << "    cmp r12, " << arity << "\n";
                out << "    jl stack_underflow\n\n";
                
                if (token.strValue == "abs") {
                    out << "    ; Absolute value\n";
                    out << "    call pop_stack  ; Get operand into xmm0\n";
                    out << "    andpd xmm0, [rel __m128d_abs_mask]\n";
                    out << "    call push_stack\n";
                }
                else if (token.strValue == "sin") {
                    out << "    ; Sine function\n";
                    out << "    call pop_stack  ; Get operand into xmm0\n";
                    out << "    sub rsp, 8  ; Align stack\n";
                    out << "    call sin\n";
                    out << "    add rsp, 8  ; Restore stack\n";
                    out << "    call push_stack\n";
                }
                else if (token.strValue == "cos") {
                    out << "    ; Cosine function\n";
                    out << "    call pop_stack  ; Get operand into xmm0\n";
                    out << "    sub rsp, 8  ; Align stack\n";
                    out << "    call cos\n";
                    out << "    add rsp, 8  ; Restore stack\n";
                    out << "    call push_stack\n";
                }
                else if (token.strValue == "tan") {
                    out << "    ; Tangent function\n";
                    out << "    call pop_stack  ; Get operand into xmm0\n";
                    out << "    sub rsp, 8  ; Align stack\n";
                    out << "    call tan\n";
                    out << "    add rsp, 8  ; Restore stack\n";
                    out << "    call push_stack\n";
                }
                else if (token.strValue == "sqrt") {
                    out << "    ; Square root\n";
                    out << "    call pop_stack  ; Get operand into xmm0\n";
                    out << "    sqrtsd xmm0, xmm0\n";
                    out << "    call push_stack\n";
                }
                break;
            }
            
            case Token::STACK_OP: {
                int arity = operatorArities[token.strValue];
                
                out << "    ; Check if we have enough operands\n";
                out << "    cmp r12, " << arity << "\n";
                out << "    jl stack_underflow\n\n";
                
                if (token.strValue == "swap") {
                    out << "    ; Swap top two stack elements\n";
                    out << "    mov rax, r12\n";
                    out << "    dec rax\n";
                    out << "    movsd xmm0, [rbp + 8*rax]\n";  // Top item
                    out << "    mov rbx, rax\n";
                    out << "    dec rbx\n";
                    out << "    movsd xmm1, [rbp + 8*rbx]\n";  // Second item
                    out << "    movsd [rbp + 8*rax], xmm1\n";
                    out << "    movsd [rbp + 8*rbx], xmm0\n";
                }
                else if (token.strValue == "dup") {
                    out << "    ; Duplicate top stack element\n";
                    out << "    call pop_stack\n";  // Get top item
                    out << "    call push_stack\n";  // Push it back
                    out << "    call push_stack\n";  // Push it again
                }
                break;
            }
        }
        
        out << "\n";
    }
    
    // Print the final result
    out << "    ; Print the final result\n";
    out << "    call pop_stack\n";
    out << "    lea rdi, [rel format]\n";
    out << "    mov rax, 1  ; One floating point argument\n";
    out << "    call printf\n\n";
    
    // Exit program
    out << "    ; Exit program\n";
    out << "    xor rdi, rdi\n";
    out << "    call exit\n\n";
    
    // Add helper functions
    out << "push_stack:\n";
    out << "    ; Push value in xmm0 to stack\n";
    out << "    mov rax, r12\n";
    out << "    movsd [rbp + 8*rax], xmm0\n";
    out << "    inc r12\n";
    out << "    ret\n\n";
    
    out << "pop_stack:\n";
    out << "    ; Pop value from stack to xmm0\n";
    out << "    dec r12\n";
    out << "    mov rax, r12\n";
    out << "    movsd xmm0, [rbp + 8*rax]\n";
    out << "    ret\n\n";
    
    // Error handlers
    out << "division_by_zero:\n";
    out << "    ; Handle division by zero error\n";
    out << "    lea rdi, [rel div_zero_msg]\n";
    out << "    xor rax, rax\n";
    out << "    call printf\n";
    out << "    mov rdi, 1  ; Exit code 1\n";
    out << "    call exit\n\n";
    
    out << "stack_underflow:\n";
    out << "    ; Handle stack underflow error\n";
    out << "    lea rdi, [rel stack_underflow_msg]\n";
    out << "    xor rax, rax\n";
    out << "    call printf\n";
    out << "    mov rdi, 2  ; Exit code 2\n";
    out << "    call exit\n\n";
    
    // Data section
    out << "section .rodata\n";
    out << "    __m128d_abs_mask dq 0x7FFFFFFFFFFFFFFF, 0x7FFFFFFFFFFFFFFF\n";
    
    // External functions for math operations
    out << "    extern floor\n";
    out << "    extern log\n";
    out << "    extern exp\n";
    out << "    extern sin\n";
    out << "    extern cos\n";
    out << "    extern tan\n";
} 