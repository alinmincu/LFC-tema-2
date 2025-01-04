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

    bool isGlobalScope = true;

    while (stream.get(ch)) {
        if (ch == '\n') {
            currentLine++;
        }

        if (isspace(ch)) {
            continue;
        }

        if (ch == '/') {
            skipComment(ch, stream);
            continue;
        }

        if (isalpha(ch) || ch == '_') {
            currentLexeme = ch;
            while (stream.get(ch) && (isalnum(ch) || ch == '_')) {
                currentLexeme += ch;
            }
            stream.unget();

            if (isKeyword(currentLexeme)) {
                addToken(TokenType::KEYWORD, currentLexeme);

                // Detectăm variabilele globale
                if (isGlobalScope && (currentLexeme == "int" || currentLexeme == "float" || currentLexeme == "string")) {
                    std::string type = currentLexeme;
                    std::string name;
                    std::string value;

                    // Citim numele variabilei
                    while (stream.get(ch) && isspace(ch)); // Ignorăm spațiile
                    if (isalpha(ch) || ch == '_') {
                        name = ch;
                        while (stream.get(ch) && (isalnum(ch) || ch == '_')) {
                            name += ch;
                        }

                        // Citim valoarea inițială, dacă există
                        if (ch == '=') {
                            while (stream.get(ch) && ch != ';') {
                                value += ch; // Construim valoarea
                            }
                        }

                        // Adăugăm variabila globală
                        addGlobalVariable(name, type, value);
                    }
                }

                // Verificăm dacă am ieșit din scope-ul global
                if (currentLexeme == "void" || currentLexeme == "int" || currentLexeme == "float") {
                    isGlobalScope = false;
                }
            }
            else {
                addToken(TokenType::IDENTIFIER, currentLexeme);
            }
        }
        else if (isdigit(ch)) {
            currentLexeme = ch;
            while (stream.get(ch) && (isdigit(ch) || ch == '.')) {
                currentLexeme += ch;
            }
            stream.unget();
            if (isNumber(currentLexeme)) {
                addToken(TokenType::NUMBER, currentLexeme);
            }
            else {
                addToken(TokenType::ERROR, currentLexeme);
            }
        }
        else if (ch == '"' || ch == '\'') {
            char quoteType = ch;
            currentLexeme = ch;
            while (stream.get(ch)) {
                currentLexeme += ch;
                if (ch == quoteType) {
                    addToken(TokenType::STRING_LITERAL, currentLexeme);
                    break;
                }
                if (ch == '\n') {
                    addToken(TokenType::ERROR, currentLexeme);
                    break;
                }
            }
        }
        else if (isOperator(ch)) {
            currentLexeme = ch;
            char nextCh;
            if (stream.get(nextCh)) {
                std::string combined = currentLexeme + nextCh;
                if (isOperator(combined)) {
                    currentLexeme = combined;
                }
                else {
                    stream.unget();
                }
            }
            addToken(TokenType::OPERATOR, currentLexeme);
        }
        else if (isDelimiter(ch)) {
            currentLexeme = ch;
            addToken(TokenType::DELIMITER, currentLexeme);
        }
        else {
            currentLexeme = ch;
            addToken(TokenType::ERROR, currentLexeme);
        }
    }
}

void Lexer::skipComment(char& ch, std::istringstream& stream) {
    if (ch == '/') {
        char nextChar;
        stream.get(nextChar);
        if (nextChar == '/') { // Linie de comentariu
            while (stream.get(ch) && ch != '\n');
        }
        else if (nextChar == '*') { // Comentariu multi-linie
            while (stream.get(ch)) {
                if (ch == '*' && stream.peek() == '/') {
                    stream.get(ch);
                    break;
                }
            }
        }
        else {
            stream.unget();
        }
    }
}

bool Lexer::isComment(char ch, std::istringstream& stream) {
    if (ch == '/') {
        char nextChar = stream.peek();
        return nextChar == '/' || nextChar == '*';
    }
    return false;
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
        "int", "float", "string", "void", "if", "else", "while", "for", "return",
        "break", "continue"
    };
    return keywords.find(lexeme) != keywords.end();
}

bool Lexer::isNumber(const std::string& lexeme) {
    std::regex numberRegex(R"(^(\d+(\.\d*)?|\.\d+)$)");
    return std::regex_match(lexeme, numberRegex);
}

const std::vector<Token>& Lexer::getTokens() const {
    return tokens;
}

const std::vector<GlobalVariable>& Lexer::getGlobalVariables() const {
    return globalVariables;
}

const std::vector<FunctionInfo>& Lexer::getFunctions() const {
    return functions;
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

void Lexer::printGlobalVariables() const {
    std::cout << "Global Variables:\n";

    if (globalVariables.empty()) {
        std::cout << "No global variables found.\n";
        return;
    }

    for (const auto& var : globalVariables) {
        std::cout << "Type: " << var.type
            << ", Name: " << var.name
            << ", Initial Value: " << (var.value.empty() ? "EMPTY" : var.value) << "\n";
    }
}

void Lexer::printFunctions() const {
    std::cout << "Functions:\n";
    for (const auto& func : functions) {
        std::cout << "Name: " << func.name << ", Type: " << func.type
            << ", Recursive: " << (func.isRecursive ? "Yes" : "No") << "\n";
        std::cout << "Parameters: " << func.parameters << "\n";
        std::cout << "Local Variables:\n";
        for (const auto& var : func.localVariables) {
            std::cout << "  " << var << "\n";
        }
        std::cout << "Control Structures:\n";
        for (const auto& control : func.controlStructures) {
            std::cout << "  " << control << "\n";
        }
        std::cout << "\n";
    }
}

void Lexer::printErrors() const {
    std::cout << "Errors:\n";
    for (const auto& token : tokens) {
        if (token.type == ERROR) {
            std::cout << "Lexical Error at line " << token.line << ": Invalid token '" << token.value << "'\n";
        }
    }
}

void Lexer::addGlobalVariable(const std::string& name, const std::string& type, const std::string& value) {
    globalVariables.push_back({ name, type, value });

    // Debugging
    std::cout << "Debug: Global variable added - Type: " << type
        << ", Name: " << name
        << ", Value: " << (value.empty() ? "EMPTY" : value) << "\n";
}
