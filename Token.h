#ifndef TOKEN_H
#define TOKEN_H

#include <string>
#include <vector>

enum TokenType {
    KEYWORD, IDENTIFIER, OPERATOR, NUMBER, STRING_LITERAL, DELIMITER, ERROR
};

struct Token {
    TokenType type;
    std::string value;
    int line;
};

#endif // TOKEN_H
