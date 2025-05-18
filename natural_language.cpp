#include "natural_language.h"
#include <sstream>
#include <queue>
#include <stack>
#include <cctype>

NaturalLanguageProcessor::NaturalLanguageProcessor() {
    // Initialize word to number mappings
    wordToNumber["zero"] = "0";
    wordToNumber["one"] = "1";
    wordToNumber["two"] = "2";
    wordToNumber["three"] = "3";
    wordToNumber["four"] = "4";
    wordToNumber["five"] = "5";
    wordToNumber["six"] = "6";
    wordToNumber["seven"] = "7";
    wordToNumber["eight"] = "8";
    wordToNumber["nine"] = "9";
    wordToNumber["ten"] = "10";
    wordToNumber["eleven"] = "11";
    wordToNumber["twelve"] = "12";
    wordToNumber["thirteen"] = "13";
    wordToNumber["fourteen"] = "14";
    wordToNumber["fifteen"] = "15";
    wordToNumber["sixteen"] = "16";
    wordToNumber["seventeen"] = "17";
    wordToNumber["eighteen"] = "18";
    wordToNumber["nineteen"] = "19";
    wordToNumber["twenty"] = "20";
    wordToNumber["thirty"] = "30";
    wordToNumber["forty"] = "40";
    wordToNumber["fifty"] = "50";
    wordToNumber["sixty"] = "60";
    wordToNumber["seventy"] = "70";
    wordToNumber["eighty"] = "80";
    wordToNumber["ninety"] = "90";
    wordToNumber["hundred"] = "100";
    wordToNumber["thousand"] = "1000";
    wordToNumber["million"] = "1000000";
    wordToNumber["billion"] = "1000000000";
    wordToNumber["pi"] = "pi";
    wordToNumber["e"] = "e";
    
    // Initialize word to operator mappings
    wordToOperator["plus"] = "+";
    wordToOperator["add"] = "+";
    wordToOperator["added"] = "+";
    wordToOperator["addition"] = "+";
    wordToOperator["sum"] = "+";
    
    wordToOperator["minus"] = "-";
    wordToOperator["subtract"] = "-";
    wordToOperator["subtracted"] = "-";
    wordToOperator["subtraction"] = "-";
    wordToOperator["difference"] = "-";
    
    wordToOperator["times"] = "*";
    wordToOperator["multiply"] = "*";
    wordToOperator["multiplied"] = "*";
    wordToOperator["multiplication"] = "*";
    wordToOperator["product"] = "*";
    
    wordToOperator["divided"] = "/";
    wordToOperator["divide"] = "/";
    wordToOperator["division"] = "/";
    wordToOperator["quotient"] = "/";
    wordToOperator["over"] = "/";
    
    wordToOperator["power"] = "^";
    wordToOperator["exponent"] = "^";
    wordToOperator["raised"] = "^";
    wordToOperator["to the power of"] = "^";
    
    wordToOperator["modulo"] = "%";
    wordToOperator["mod"] = "%";
    wordToOperator["remainder"] = "%";
    
    wordToOperator["factorial"] = "!";
    
    wordToOperator["sin"] = "sin";
    wordToOperator["sine"] = "sin";
    wordToOperator["cos"] = "cos";
    wordToOperator["cosine"] = "cos";
    wordToOperator["tan"] = "tan";
    wordToOperator["tangent"] = "tan";
    wordToOperator["sqrt"] = "sqrt";
    wordToOperator["square root"] = "sqrt";
    wordToOperator["square root of"] = "sqrt";
    
    // Operator precedence (higher number = higher precedence)
    operatorPrecedence["+"] = 1;
    operatorPrecedence["-"] = 1;
    operatorPrecedence["*"] = 2;
    operatorPrecedence["/"] = 2;
    operatorPrecedence["%"] = 2;
    operatorPrecedence["^"] = 3;
    operatorPrecedence["!"] = 4;
    operatorPrecedence["sin"] = 4;
    operatorPrecedence["cos"] = 4;
    operatorPrecedence["tan"] = 4;
    operatorPrecedence["sqrt"] = 4;
    
    // Right associative operators
    rightAssociative["^"] = true;
}

std::vector<std::string> NaturalLanguageProcessor::tokenizeNaturalLanguage(const std::string& naturalExpression) {
    std::vector<std::string> result;
    std::istringstream iss(naturalExpression);
    std::string word;
    
    // Convert to lowercase and split by spaces
    while (iss >> word) {
        // Convert to lowercase
        std::transform(word.begin(), word.end(), word.begin(), 
                       [](unsigned char c){ return std::tolower(c); });
        
        // Remove punctuation from the word
        word.erase(std::remove_if(word.begin(), word.end(), 
                                 [](char c) { return std::ispunct(c) && c != '.'; }), 
                  word.end());
        
        if (!word.empty()) {
            result.push_back(word);
        }
    }
    
    // Process multi-word phrases like "to the power of"
    for (size_t i = 0; i < result.size(); ++i) {
        // Check for "to the power of" (needs 4 consecutive words)
        if (i + 3 < result.size() && result[i] == "to" && result[i+1] == "the" && 
            result[i+2] == "power" && result[i+3] == "of") {
            result[i] = "to the power of";
            result.erase(result.begin() + i + 1, result.begin() + i + 4);
        }
        // Check for "square root of" (needs 3 consecutive words)
        else if (i + 2 < result.size() && result[i] == "square" && result[i+1] == "root" && 
                 result[i+2] == "of") {
            result[i] = "square root of";
            result.erase(result.begin() + i + 1, result.begin() + i + 3);
        }
        // Check for "square root" (needs 2 consecutive words)
        else if (i + 1 < result.size() && result[i] == "square" && result[i+1] == "root") {
            result[i] = "square root";
            result.erase(result.begin() + i + 1, result.begin() + i + 2);
        }
    }
    
    return result;
}

std::string NaturalLanguageProcessor::convertToRPN(const std::string& naturalExpression) {
    // Tokenize the natural language expression
    std::vector<std::string> tokens = tokenizeNaturalLanguage(naturalExpression);
    
    // Convert words to numbers and operators
    std::vector<std::string> processedTokens;
    
    for (const auto& token : tokens) {
        // Check if it's a number word
        if (wordToNumber.find(token) != wordToNumber.end()) {
            processedTokens.push_back(wordToNumber[token]);
        }
        // Check if it's an operator word
        else if (wordToOperator.find(token) != wordToOperator.end()) {
            processedTokens.push_back(wordToOperator[token]);
        }
        // Check if it's already a number or symbol
        else if (std::isdigit(token[0]) || token[0] == '.' || token[0] == '-' || 
                 token[0] == '+' || token[0] == '*' || token[0] == '/' || 
                 token[0] == '^' || token[0] == '%' || token[0] == '!' ||
                 token == "(" || token == ")") {
            processedTokens.push_back(token);
        }
        // Skip words like "and", "by", "with", etc.
        else if (token == "and" || token == "by" || token == "with" || 
                 token == "then" || token == "to" || token == "equals" ||
                 token == "is" || token == "the" || token == "of") {
            continue;
        }
        // Unknown word - could be a variable or function name
        else {
            processedTokens.push_back(token);
        }
    }
    
    // Convert from infix to RPN (Reverse Polish Notation)
    return infixToRPN(processedTokens);
}

std::string NaturalLanguageProcessor::infixToRPN(const std::vector<std::string>& tokens) {
    std::ostringstream result;
    std::stack<std::string> operatorStack;
    
    for (const auto& token : tokens) {
        // If token is a number, output it directly
        if (std::isdigit(token[0]) || (token[0] == '-' && token.size() > 1 && std::isdigit(token[1])) ||
            token == "pi" || token == "e") {
            result << token << " ";
        }
        // If it's a function or ( push onto stack
        else if (token == "(" || token == "sin" || token == "cos" || token == "tan" || token == "sqrt") {
            operatorStack.push(token);
        }
        // If it's a closing parenthesis
        else if (token == ")") {
            while (!operatorStack.empty() && operatorStack.top() != "(") {
                result << operatorStack.top() << " ";
                operatorStack.pop();
            }
            if (!operatorStack.empty() && operatorStack.top() == "(") {
                operatorStack.pop(); // Discard the opening parenthesis
            }
        }
        // If it's an operator
        else if (token == "+" || token == "-" || token == "*" || token == "/" || token == "^" || 
                 token == "%" || token == "!") {
            while (!operatorStack.empty() && operatorStack.top() != "(" &&
                   ((rightAssociative.find(token) == rightAssociative.end() || !rightAssociative[token]) &&
                    operatorPrecedence[operatorStack.top()] >= operatorPrecedence[token]) ||
                   (rightAssociative.find(token) != rightAssociative.end() && rightAssociative[token] &&
                    operatorPrecedence[operatorStack.top()] > operatorPrecedence[token])) {
                result << operatorStack.top() << " ";
                operatorStack.pop();
            }
            operatorStack.push(token);
        }
    }
    
    // Pop any remaining operators from the stack
    while (!operatorStack.empty()) {
        if (operatorStack.top() != "(") {
            result << operatorStack.top() << " ";
        }
        operatorStack.pop();
    }
    
    return result.str();
} 