#include "lexer.h"
#include <cctype>
#include <iostream>
#include <sstream>
#include <unordered_set>
#include <regex>

Lexer::Lexer(const std::string& code) : code(code), currentIndex(0), currentLine(1) {}

void Lexer::tokenize() {
    std::istringstream stream(code);
    char ch;
    std::string currentLexeme;

    while (stream.get(ch)) {
        if (ch == '\n') {
            currentLine++;
        }

        if (isspace(ch)) {
            continue; // Ignorăm spațiile
        }

        if (isalpha(ch) || ch == '_') {  // Identificatori și cuvinte cheie
            currentLexeme = ch;
            while (stream.get(ch) && (isalnum(ch) || ch == '_')) {
                currentLexeme += ch;
            }
            stream.unget();

            if (isKeyword(currentLexeme)) {
                addToken(TokenType::KEYWORD, currentLexeme);
            }
            else {
                addToken(TokenType::IDENTIFIER, currentLexeme);
            }
        }
        else if (isdigit(ch)) {  // Numere
            currentLexeme = ch;
            while (stream.get(ch) && (isdigit(ch) || ch == '.')) {
                currentLexeme += ch;
            }
            stream.unget(); // Ultimul caracter care nu e parte din număr
            if (isNumber(currentLexeme)) {
                addToken(TokenType::NUMBER, currentLexeme);
            }
            else {
                addToken(TokenType::ERROR, currentLexeme);
            }
        }
        else if (ch == '"' || ch == '\'') {  // Șiruri de caractere
            char quoteType = ch;  // Stochează tipul ghilimelei
            currentLexeme = ch;
            while (stream.get(ch)) {
                currentLexeme += ch;
                if (ch == quoteType) {  // Închidere ghilimele
                    addToken(TokenType::STRING_LITERAL, currentLexeme);
                    break;
                }
                if (ch == '\n') {  // Eroare: ghilimelele nu s-au închis pe aceeași linie
                    addToken(TokenType::ERROR, currentLexeme);
                    break;
                }
            }
            if (currentLexeme.back() != quoteType) {  // Dacă nu s-au închis corect
                addToken(TokenType::ERROR, currentLexeme);
            }
        }
        else if (isOperator(ch)) {  // Operatorii
            currentLexeme = ch;
            char nextCh;
            if (stream.get(nextCh)) {
                std::string combined = currentLexeme + nextCh;
                if (isOperator(combined)) {  // Operator format din două caractere (e.g., ==, !=, etc.)
                    currentLexeme = combined;
                }
                else {
                    stream.unget();  // Nu face parte din operator
                }
            }
            addToken(TokenType::OPERATOR, currentLexeme);
        }
        else if (isDelimiter(ch)) {  // Delimitatori
            currentLexeme = ch;
            addToken(TokenType::DELIMITER, currentLexeme);
        }
        else {  // Altele, le considerăm erori
            currentLexeme = ch;
            addToken(TokenType::ERROR, currentLexeme);
        }
    }
}

const std::vector<Token>& Lexer::getTokens() const {
    return tokens;
}

void Lexer::printTokens() const {
    for (const auto& token : tokens) {
        std::cout << "(" << (token.type == KEYWORD ? "KEYWORD" :
            token.type == IDENTIFIER ? "IDENTIFIER" :
            token.type == OPERATOR ? "OPERATOR" :
            token.type == NUMBER ? "NUMBER" :
            token.type == STRING_LITERAL ? "STRING_LITERAL" :
            token.type == DELIMITER ? "DELIMITER" : "ERROR")
            << ", " << token.value << ", " << token.line << ")\n";
    }
}

void Lexer::addToken(TokenType type, const std::string& value) {
    tokens.push_back({ type, value, currentLine });
}

bool Lexer::isOperator(char c) {
    return c == '+' || c == '-' || c == '*' || c == '/' || c == '=' ||
        c == '>' || c == '<' || c == '!' || c == '&' || c == '|';
}

bool Lexer::isOperator(const std::string& str) {
    static const std::unordered_set<std::string> operators = {
        "+", "-", "*", "/", "=", "==", "!=", "<", ">", "<=", ">=", "&&", "||",
        "!", "+=", "-=", "*=", "/="
    };
    return operators.find(str) != operators.end();
}

bool Lexer::isDelimiter(char c) {
    return c == '(' || c == ')' || c == '{' || c == '}' || c == ',' || c == ';';
}

bool Lexer::isKeyword(const std::string& lexeme) {
    static const std::unordered_set<std::string> keywords = {
        "int", "float", "string", "if", "else", "while", "for", "void",
        "return", "break", "continue"
    };
    return keywords.find(lexeme) != keywords.end();
}

bool Lexer::isNumber(const std::string& lexeme) {
    std::regex numberRegex(R"(^(\d+(\.\d*)?|\.\d+)$)"); // Regula pentru numere întregi sau zecimale
    return std::regex_match(lexeme, numberRegex);
}
