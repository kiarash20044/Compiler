#ifndef COMPILER_H
#define COMPILER_H

#include <string>
#include <vector>
#include <map>
#include <cmath>
#include <stdexcept>
#include <sstream>

class Token {
public:
    enum Type {
        NUMBER,
        OPERATOR,
        FUNCTION,
        CONSTANT,
        STACK_OP
    };
    
    Token(Type type, const std::string& value) : type(type), strValue(value) {
        if (type == NUMBER) {
            numValue = std::stod(value);
        }
    }
    
    Type type;
    std::string strValue;
    double numValue;
};

class Compiler {
public:
    Compiler();
    void compile(const std::string& expression, const std::string& outputFile);
    std::string compileToString(const std::string& expression);
    
    // Made public for direct testing
    std::vector<Token> tokenize(const std::string& expression);
    void generateAssembly(const std::vector<Token>& tokens, std::ostream& out);
    
    std::map<std::string, int> operatorArities;
    std::map<std::string, double> constants;
};

#endif // COMPILER_H 