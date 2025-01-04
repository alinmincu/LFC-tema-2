//#include "lexer.h"
//#include "parser.h"
//#include "SemanticAnalyzer.h"
//#include <fstream>
//#include <sstream>
//
//int main() {
//    std::ifstream inputFile("in.txt");
//    if (!inputFile.is_open()) {
//        std::cerr << "Could not open file!" << std::endl;
//        return 1;
//    }
//
//    std::string code((std::istreambuf_iterator<char>(inputFile)), std::istreambuf_iterator<char>());
//    inputFile.close();
//
//    // Lexer
//    Lexer lexer(code);
//    lexer.tokenize();
//    lexer.printTokens();
//    lexer.printGlobalVariables();
//
//    // Parser
//    Parser parser(lexer.getTokens());
//    parser.parse();
//    parser.printErrors();
//
//    // Semantic Analyzer
//    SemanticAnalyzer semanticAnalyzer(lexer.getGlobalVariables(), lexer.getFunctions());
//    semanticAnalyzer.analyze();
//    semanticAnalyzer.printErrors();
//
//    lexer.printFunctions();
//
//    return 0;
//}

#include <fstream>
#include <string>
#include <unordered_set>
#include <regex>
#include <iostream>
#include <sstream>

bool hasMainFunction(const std::string& code) {
    return code.find("int main(") != std::string::npos;
}

bool isInitializationValid(const std::string& type, const std::string& value) {
    if (type == "string") {
        return value.front() == '"' && value.back() == '"';
    }
    if (type == "int" || type == "float") {
        return std::regex_match(value, std::regex("^[-+]?\\d*\\.?\\d+$"));
    }
    return false;
}

bool isAlreadyDeclared(const std::string& varName, const std::unordered_set<std::string>& declaredVars) {
    return declaredVars.find(varName) != declaredVars.end();
}

std::string removeComments(const std::string& code) {
    std::string cleanedCode = code;
    std::regex singleLineCommentRegex(R"(//.*?$)");
    std::regex multiLineCommentRegex(R"(/\*[\s\S]*?\*/)");

    cleanedCode = std::regex_replace(cleanedCode, singleLineCommentRegex, "");
    cleanedCode = std::regex_replace(cleanedCode, multiLineCommentRegex, "");

    return cleanedCode;
}

std::string removeInvalidDeclarations(const std::string& code, const std::unordered_set<std::string>& invalidLines) {
    std::string result;
    std::istringstream stream(code);
    std::string line;

    while (std::getline(stream, line)) {
        if (invalidLines.find(line) == invalidLines.end()) {
            result += line + "\n";
        }
    }
    return result;
}

int main() {
    std::ifstream inputFile("input.txt");
    if (!inputFile) {
        std::cerr << "Error: Could not open input file.\n";
        return 1;
    }

    std::string code((std::istreambuf_iterator<char>(inputFile)), std::istreambuf_iterator<char>());
    inputFile.close();

    std::ofstream outputFile("output.txt");
    if (!outputFile) {
        std::cerr << "Error: Could not open output file.\n";
        return 1;
    }

    std::unordered_set<std::string> declaredVariables;
    std::unordered_set<std::string> invalidLines;

    if (!hasMainFunction(code)) {
        outputFile << "Error: main() function not found.\n";
    }

    std::string cleanedCode = removeComments(code);
    std::regex varDeclarationRegex("(int|float|string)\\s+([a-zA-Z_][a-zA-Z0-9_]*)\\s*=\\s*([^;]+);");
    std::smatch match;
    std::string::const_iterator searchStart(cleanedCode.cbegin());

    while (std::regex_search(searchStart, cleanedCode.cend(), match, varDeclarationRegex)) {
        std::string type = match[1];
        std::string varName = match[2];
        std::string value = match[3];

        if (isAlreadyDeclared(varName, declaredVariables)) {
            outputFile << "Error: Variable " << varName << " is already declared.\n";
            invalidLines.insert(match.str());
        }
        else {
            if (isInitializationValid(type, value)) {
                declaredVariables.insert(varName);
            }
            else {
                outputFile << "Error: Invalid initialization for variable " << varName << " with value " << value << ".\n";
                invalidLines.insert(match.str());
            }
        }

        searchStart = match.suffix().first;
    }

    cleanedCode = removeInvalidDeclarations(cleanedCode, invalidLines);

    outputFile << "\nCleaned Code:\n" << cleanedCode << "\n";
    outputFile.close();

    std::cout << "Code analysis completed. Check 'output.txt' for results.\n";
    return 0;
}
