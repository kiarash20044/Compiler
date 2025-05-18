#ifndef NATURAL_LANGUAGE_H
#define NATURAL_LANGUAGE_H

#include <string>
#include <map>
#include <vector>
#include <algorithm>
#include <regex>

class NaturalLanguageProcessor {
public:
    NaturalLanguageProcessor();
    
    // Convert natural language to RPN
    std::string convertToRPN(const std::string& naturalExpression);
    
private:
    // Helper function to preprocess and tokenize natural language
    std::vector<std::string> tokenizeNaturalLanguage(const std::string& naturalExpression);
    
    // Convert infix to RPN (Shunting Yard algorithm)
    std::string infixToRPN(const std::vector<std::string>& tokens);
    
    // Maps for number words, operators in natural language
    std::map<std::string, std::string> wordToNumber;
    std::map<std::string, std::string> wordToOperator;
    std::map<std::string, int> operatorPrecedence;
    std::map<std::string, bool> rightAssociative;
};

#endif // NATURAL_LANGUAGE_H 