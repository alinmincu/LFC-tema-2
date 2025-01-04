#include "parser.h"

Parser::Parser(const std::vector<Token>& tokens) : tokens(tokens), currentIndex(0) {}

void Parser::parse() {
    while (currentIndex < tokens.size()) {
        Token token = peek();

        if (token.type == KEYWORD && (token.value == "int" || token.value == "float" || token.value == "string")) {
            parseVariableDeclaration();
        }
        else if (token.type == KEYWORD && token.value == "void") {
            parseFunction();
        }
        else if (token.type == KEYWORD && (token.value == "if" || token.value == "while" || token.value == "for")) {
            parseControlStructure();
        }
        else {
            errors.push_back("Unexpected token: " + token.value + " at line " + std::to_string(token.line));
            advance();
        }
    }
}

void Parser::parseFunction() {
    advance(); // Consuma "void"
    if (!match(IDENTIFIER)) {
        errors.push_back("Expected function name at line " + std::to_string(peek().line));
    }

    if (!match(DELIMITER) || peek().value != "(") {
        errors.push_back("Expected '(' after function name at line " + std::to_string(peek().line));
    }
    else {
        advance(); // Consuma '('
    }

    while (peek().type != DELIMITER || peek().value != ")") {
        if (!match(KEYWORD) || !match(IDENTIFIER)) {
            errors.push_back("Expected parameter type and name in function declaration at line " + std::to_string(peek().line));
        }

        if (peek().value == ",") {
            advance();
        }
    }
    advance(); // Consuma ')'

    if (!match(DELIMITER) || peek().value != "{") {
        errors.push_back("Expected '{' after function declaration at line " + std::to_string(peek().line));
    }
    else {
        advance(); // Consuma '{'
    }

    while (peek().value != "}") {
        parse();
    }

    if (!match(DELIMITER) || peek().value != "}") {
        errors.push_back("Expected '}' to close function at line " + std::to_string(peek().line));
    }
    else {
        advance();
    }
}

void Parser::parseControlStructure() {
    Token control = advance(); // Consuma "if", "while" sau "for"

    if (!match(DELIMITER) || peek().value != "(") {
        errors.push_back("Expected '(' after " + control.value + " at line " + std::to_string(peek().line));
    }
    else {
        advance(); // Consuma '('
    }

    parseExpression();

    if (!match(DELIMITER) || peek().value != ")") {
        errors.push_back("Expected ')' after condition at line " + std::to_string(peek().line));
    }
    else {
        advance(); // Consuma ')'
    }

    if (!match(DELIMITER) || peek().value != "{") {
        errors.push_back("Expected '{' after " + control.value + " condition at line " + std::to_string(peek().line));
    }
    else {
        advance(); // Consuma '{'
    }

    while (peek().value != "}") {
        parse();
    }

    if (!match(DELIMITER) || peek().value != "}") {
        errors.push_back("Expected '}' to close " + control.value + " block at line " + std::to_string(peek().line));
    }
    else {
        advance();
    }
}

void Parser::parseVariableDeclaration() {
    Token typeToken = advance(); // Tipul variabilei
    if (typeToken.type != KEYWORD || (typeToken.value != "int" && typeToken.value != "float" && typeToken.value != "string")) {
        errors.push_back("Expected variable type at line " + std::to_string(typeToken.line));
        return;
    }

    Token nameToken = advance(); // Numele variabilei
    if (nameToken.type != IDENTIFIER) {
        errors.push_back("Expected variable name at line " + std::to_string(nameToken.line));
        return;
    }

    Token nextToken = peek(); // Verificăm ce urmează
    if (nextToken.type == OPERATOR && nextToken.value == "=") {
        advance(); // Consuma "="
        parseExpression(); // Procesăm expresia
    }

    nextToken = advance(); // Trebuie să fie un delimitator ";"
    if (nextToken.type != DELIMITER || nextToken.value != ";") {
        errors.push_back("Expected ';' after variable declaration at line " + std::to_string(nextToken.line));
    }
}

void Parser::parseExpression() {
    // Simplă verificare pentru expresii de bază
    while (peek().type == IDENTIFIER || peek().type == NUMBER || peek().type == OPERATOR) {
        advance();
    }
}

Token Parser::peek() const {
    return currentIndex < tokens.size() ? tokens[currentIndex] : Token{ ERROR, "", -1 };
}

Token Parser::advance() {
    return currentIndex < tokens.size() ? tokens[currentIndex++] : Token{ ERROR, "", -1 };
}

bool Parser::match(TokenType type) {
    if (peek().type == type) {
        advance();
        return true;
    }
    return false;
}

const std::vector<std::string>& Parser::getErrors() const {
    return errors;
}

void Parser::printErrors() const {
    std::cout << "Syntax Errors:\n";
    for (const auto& error : errors) {
        std::cout << error << "\n";
    }
}
