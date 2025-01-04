#ifndef PARSER_H
#define PARSER_H

#include "token.h"
#include "lexer.h"
#include <vector>
#include <string>
#include <iostream>

class Parser {
public:
    explicit Parser(const std::vector<Token>& tokens);
    void parse();
    const std::vector<std::string>& getErrors() const;
    void printErrors() const;

private:
    const std::vector<Token>& tokens;
    size_t currentIndex;
    std::vector<std::string> errors;

    void parseFunction();
    void parseControlStructure();
    void parseVariableDeclaration();
    void parseExpression();
    Token peek() const;
    Token advance();
    bool match(TokenType type);
};

#endif // PARSER_H
