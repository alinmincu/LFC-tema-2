#ifndef LEXER_H
#define LEXER_H

#include "token.h"
#include <vector>
#include <string>
#include <iostream>

class Lexer {
public:
    Lexer(const std::string& code);
    void tokenize();
    const std::vector<Token>& getTokens() const;
    void printTokens() const;

private:
    std::string code;
    std::vector<Token> tokens;
    int currentIndex;
    int currentLine;

    void addToken(TokenType type, const std::string& value);
    void processKeywordsAndIdentifiers(const std::string& lexem);
    void processOperatorsAndDelimiters(const std::string& lexem);
    void processNumbersAndLiterals(const std::string& lexem);
    void processErrors(const std::string& lexem);
    void skipWhitespace();
	bool isOperator(char c);
	bool isOperator(const std::string& str);
	bool isDelimiter(char c);
    bool isKeyword(const std::string& lexeme);
    bool isNumber(const std::string& lexeme);
    bool isStringLiteral(const std::string& lexeme);
};

#endif // LEXER_H
