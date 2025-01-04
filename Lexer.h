#ifndef LEXER_H
#define LEXER_H

#include "token.h"
#include <vector>
#include <string>
#include <iostream>

struct GlobalVariable {
    std::string name;
    std::string type;
    std::string value;
};

struct FunctionInfo {
    std::string name;
    std::string type;
    std::string parameters;
    bool isRecursive;
    std::vector<std::string> localVariables;
    std::vector<std::string> controlStructures;
};

class Lexer {
public:
    Lexer(const std::string& code);
    void tokenize();
    const std::vector<Token>& getTokens() const;
    const std::vector<GlobalVariable>& getGlobalVariables() const;
    const std::vector<FunctionInfo>& getFunctions() const;
    void printTokens() const;
    void printGlobalVariables() const;
    void printFunctions() const;
    void printErrors() const;

private:
    std::string code;
    std::vector<Token> tokens;
    std::vector<GlobalVariable> globalVariables;
    std::vector<FunctionInfo> functions;
    int currentIndex;
    int currentLine;

    void addToken(TokenType type, const std::string& value);
    void collectVariableOrFunction(const std::string& identifier, std::istringstream& stream);
	void addGlobalVariable(const std::string& name, const std::string& type, const std::string& value);
    void addFunction(const std::string& name, std::istringstream& stream);
    void skipComment(char& ch, std::istringstream& stream);
    bool isComment(char ch, std::istringstream& stream);
    bool isOperator(char c);
    bool isOperator(const std::string& str);
    bool isDelimiter(char c);
    bool isKeyword(const std::string& lexeme);
    bool isNumber(const std::string& lexeme);
};

#endif // LEXER_H
