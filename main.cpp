#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>
#include <filesystem>
#include "compiler.h"
#include "natural_language.h"

// Function to sanitize expression for use as filename
std::string sanitizeForFilename(const std::string& expression) {
    std::string result = expression;
    
    // Replace invalid characters with underscores
    for (char& c : result) {
        if (c == ' ' || c == '/' || c == '\\' || c == ':' || c == '*' || 
            c == '?' || c == '"' || c == '<' || c == '>' || c == '|') {
            c = '_';
        }
    }
    
    // Limit the length to avoid excessively long filenames
    if (result.length() > 50) {
        result = result.substr(0, 50);
    }
    
    // Add .asm extension if not present
    if (result.size() < 4 || result.substr(result.size() - 4) != ".asm") {
        result += ".asm";
    }
    
    return result;
}

// Function to determine if expression is in natural language
bool isNaturalLanguage(const std::string& expression) {
    // Check if the expression contains words instead of just symbols
    for (size_t i = 0; i < expression.size(); i++) {
        // Skip spaces
        if (expression[i] == ' ') continue;
        
        // If we find alphabetic characters that are not 'e' or 'E' (scientific notation)
        // and not function names (sin, cos, etc.), it's likely natural language
        if (std::isalpha(expression[i]) && 
            expression[i] != 'e' && expression[i] != 'E' &&
            // Check for common functions
            expression.substr(i, 3) != "sin" && 
            expression.substr(i, 3) != "cos" && 
            expression.substr(i, 3) != "tan" && 
            expression.substr(i, 4) != "sqrt" &&
            expression.substr(i, 3) != "abs" &&
            expression.substr(i, 3) != "dup" &&
            expression.substr(i, 4) != "swap" &&
            // Check for constants
            expression.substr(i, 2) != "pi" && 
            expression.substr(i, 1) != "e") {
            return true;
        }
    }
    
    return false;
}

void printUsage() {
    std::cout << "Math Compiler - Converts RPN mathematical expressions to assembly\n";
    std::cout << "Usage:\n";
    std::cout << "  math-compiler                  (start in interactive mode)\n";
    std::cout << "  math-compiler <expression> [output_file]\n";
    std::cout << "  math-compiler -f <input_file> [output_file]\n";
    std::cout << "Examples:\n";
    std::cout << "  math-compiler \"3 4 +\"\n";
    std::cout << "  math-compiler \"pi 2 * sin\" output.asm\n";
    std::cout << "  math-compiler \"one plus two\" (natural language)\n";
    std::cout << "  math-compiler -f input.txt output.asm\n";
}

void interactiveMode() {
    std::cout << "Math Compiler Interactive Mode\n";
    std::cout << "==============================\n";
    std::cout << "Enter RPN expressions or natural language to convert to assembly.\n";
    std::cout << "Examples:\n";
    std::cout << "  3 4 +         (RPN for 3 + 4)\n";
    std::cout << "  pi 2 * sin    (RPN for sin(pi * 2))\n";
    std::cout << "  one plus two  (natural language)\n";
    std::cout << "Enter 'exit' to quit.\n\n";
    
    Compiler compiler;
    NaturalLanguageProcessor nlp;
    std::string expression;
    
    // Ensure output directory exists
    std::filesystem::create_directories("output");
    
    while (true) {
        std::cout << "Expression: ";
        std::getline(std::cin, expression);
        
        if (expression == "exit" || expression == "quit") {
            break;
        }
        
        if (expression.empty()) {
            continue;
        }
        
        try {
            // Check if it's natural language and convert if needed
            std::string rpnExpression = expression;
            if (isNaturalLanguage(expression)) {
                rpnExpression = nlp.convertToRPN(expression);
                std::cout << "Converted to RPN: " << rpnExpression << std::endl;
            }
            
            // Create output filename based on expression
            std::string outputFile = "output/" + sanitizeForFilename(expression);
            
            // First get the assembly as a string
            std::string assembly = compiler.compileToString(rpnExpression);
            
            // Display the assembly
            std::cout << "\n===== GENERATED ASSEMBLY =====\n";
            std::cout << assembly;
            std::cout << "==============================\n\n";
            
            // Also save to file
            compiler.compile(rpnExpression, outputFile);
            std::cout << "Assembly saved to " << outputFile << std::endl;
        } catch (const std::exception& e) {
            std::cerr << "Compilation error: " << e.what() << std::endl;
        }
        std::cout << std::endl;
    }
}

int main(int argc, char* argv[]) {
    // Ensure output directory exists
    std::filesystem::create_directories("output");
    
    if (argc < 2) {
        interactiveMode();
        return 0;
    }

    std::string expression;
    std::string outputFile;
    
    if (std::string(argv[1]) == "-f" && argc >= 3) {
        // Read from file
        std::ifstream inFile(argv[2]);
        if (!inFile) {
            std::cerr << "Error: Could not open input file: " << argv[2] << std::endl;
            return 1;
        }
        
        std::string line;
        while (std::getline(inFile, line)) {
            expression += line + " ";
        }
        
        if (argc >= 4) {
            outputFile = argv[3];
        } else {
            // Create output filename based on input filename
            std::string inputFilename = std::string(argv[2]);
            size_t lastSlash = inputFilename.find_last_of("/\\");
            if (lastSlash != std::string::npos) {
                inputFilename = inputFilename.substr(lastSlash + 1);
            }
            outputFile = "output/" + sanitizeForFilename(expression);
        }
    } else {
        // Expression from command line
        expression = argv[1];
        
        if (argc >= 3) {
            outputFile = argv[2];
        } else {
            outputFile = "output/" + sanitizeForFilename(expression);
        }
    }

    Compiler compiler;
    NaturalLanguageProcessor nlp;
    
    try {
        // Check if it's natural language and convert if needed
        std::string rpnExpression = expression;
        if (isNaturalLanguage(expression)) {
            rpnExpression = nlp.convertToRPN(expression);
            std::cout << "Converted to RPN: " << rpnExpression << std::endl;
        }
        
        // If using command line mode, also show the assembly
        std::string assembly = compiler.compileToString(rpnExpression);
        
        // Display the assembly
        std::cout << "\n===== GENERATED ASSEMBLY =====\n";
        std::cout << assembly;
        std::cout << "==============================\n\n";
        
        compiler.compile(rpnExpression, outputFile);
        std::cout << "Assembly saved to " << outputFile << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Compilation error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
} 