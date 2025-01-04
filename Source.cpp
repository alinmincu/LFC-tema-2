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
#include <vector>

// Struct pentru variabile și funcții
struct Variable {
    std::string type;
    std::string name;
    std::string value;
    int line;
};

struct Function {
    std::string returnType;
    std::string name;
    std::vector<Variable> parameters;
    std::vector<Variable> localVariables;
    int line;
};

std::string removeInvalidDeclarations(const std::string& code, const std::unordered_set<std::string>& invalidLines) {
    std::string result;
    std::istringstream stream(code);
    std::string line;

    while (std::getline(stream, line)) {
        // Adaugă liniile care NU sunt în lista invalidă
        if (invalidLines.find(line) == invalidLines.end()) {
            result += line + "\n";
        }
    }
    return result;
}

// Verifică dacă inițializarea este validă
bool isInitializationValid(const std::string& type, const std::string& value) {
    if (type == "string") {
        return value.front() == '"' && value.back() == '"';
    }
    if (type == "int" || type == "float") {
        return std::regex_match(value, std::regex("^[-+]?\\d*\\.?\\d+$"));
    }
    return false;
}

// Elimină comentariile
std::string removeComments(const std::string& code) {
    std::string cleanedCode = code;
    std::regex singleLineCommentRegex(R"(//.*?$)");
    std::regex multiLineCommentRegex(R"(/\*[\s\S]*?\*/)");

    cleanedCode = std::regex_replace(cleanedCode, singleLineCommentRegex, "");
    cleanedCode = std::regex_replace(cleanedCode, multiLineCommentRegex, "");

    return cleanedCode;
}

// Analizează tokenurile
void analyzeTokens(const std::string& code, std::ofstream& outputFile) {
    std::regex tokenRegex(R"((int|float|string|void|if|else|for|while|return|[a-zA-Z_][a-zA-Z0-9_]*|[-+*/=<>!&|;{}(),]|\d+|".*?"))");
    std::smatch match;
    std::istringstream stream(code);
    std::string line;
    int lineNumber = 0;

    outputFile << "Lexical Units:\n";
    while (std::getline(stream, line)) {
        ++lineNumber;
        auto searchStart = line.cbegin();
        while (std::regex_search(searchStart, line.cend(), match, tokenRegex)) {
            outputFile << "(" << "TOKEN" << ", " << match.str() << ", line " << lineNumber << ")\n";
            searchStart = match.suffix().first;
        }
    }
}

// Analizează variabilele globale
void analyzeGlobalVariables(const std::string& code, std::vector<Variable>& globalVariables, std::ofstream& outputFile) {
    std::regex globalVarRegex(R"((int|float|string)\s+([a-zA-Z_][a-zA-Z0-9_]*)\s*=\s*([^;]+);)");
    std::smatch match;
    std::istringstream stream(code);
    std::string line;
    bool insideFunction = false;

    while (std::getline(stream, line)) {
        // Verificăm dacă suntem într-o funcție
        if (std::regex_search(line, std::regex(R"((int|float|string|void)\s+[a-zA-Z_][a-zA-Z0-9_]*\s*\()"))) {
            insideFunction = true;
        }
        if (line.find("}") != std::string::npos) {
            insideFunction = false;
        }

        if (!insideFunction && std::regex_search(line, match, globalVarRegex)) {
            globalVariables.push_back({ match[1], match[2], match[3], 0 });
        }
    }

    outputFile << "\nGlobal Variables:\n";
    for (const auto& var : globalVariables) {
        outputFile << var.type << " " << var.name << " = " << var.value << "\n";
    }
}

// Analizează funcțiile
void analyzeFunctions(const std::string& code, std::vector<Function>& functions, std::ofstream& outputFile) {
    std::regex functionRegex("(int|float|string|void)\\s+([a-zA-Z_][a-zA-Z0-9_]*)\\s*\\(([^)]*)\\)\\s*\\{");
    std::smatch match;
    std::string::const_iterator searchStart(code.cbegin());

    while (std::regex_search(searchStart, code.cend(), match, functionRegex)) {
        Function func;
        func.returnType = match[1];
        func.name = match[2];
        func.line = 0;

        // Extrage parametrii funcției
        std::string params = match[3];
        std::istringstream paramStream(params);
        std::string param;
        while (std::getline(paramStream, param, ',')) {
            std::regex paramRegex("(int|float|string)\\s+([a-zA-Z_][a-zA-Z0-9_]*)");
            std::smatch paramMatch;
            if (std::regex_match(param, paramMatch, paramRegex)) {
                func.parameters.push_back({ paramMatch[1], paramMatch[2], "", 0 });
            }
        }
        functions.push_back(func);
        searchStart = match.suffix().first;
    }

    outputFile << "\nFunctions:\n";
    for (const auto& func : functions) {
        outputFile << func.returnType << " " << func.name << "(";
        for (size_t i = 0; i < func.parameters.size(); ++i) {
            outputFile << func.parameters[i].type << " " << func.parameters[i].name;
            if (i < func.parameters.size() - 1) outputFile << ", ";
        }
        outputFile << ")\n";
    }
    outputFile << "\n";
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

    std::ofstream cleanedOutputFile("output_cleaned.txt");
    if (!cleanedOutputFile) {
        std::cerr << "Error: Could not open cleaned output file.\n";
        return 1;
    }

    std::unordered_set<std::string> declaredVariables;
    std::unordered_set<std::string> invalidLines;
    std::vector<Variable> globalVariables;
    std::vector<Function> functions;

    std::string cleanedCode = removeComments(code);

    // Analizează variabilele globale
    analyzeGlobalVariables(cleanedCode, globalVariables, outputFile);

    // Analizează funcțiile
    analyzeFunctions(cleanedCode, functions, outputFile);

    // Analizează tokenurile
    analyzeTokens(cleanedCode, outputFile);

    // Detectează erori și generează cod curățat
    std::regex varDeclarationRegex("(int|float|string)\\s+([a-zA-Z_][a-zA-Z0-9_]*)\\s*=\\s*([^;]+);");
    std::smatch match;
    std::string::const_iterator searchStart(cleanedCode.cbegin());

    while (std::regex_search(searchStart, cleanedCode.cend(), match, varDeclarationRegex)) {
        std::string type = match[1];
        std::string varName = match[2];
        std::string value = match[3];

        if (declaredVariables.find(varName) != declaredVariables.end()) {
            cleanedOutputFile << "Error: Variable " << varName << " is already declared.\n";
            invalidLines.insert(match.str());
        }
        else {
            if (isInitializationValid(type, value)) {
                declaredVariables.insert(varName);
            }
            else {
                cleanedOutputFile << "Error: Invalid initialization for variable " << varName << " with value " << value << ".\n";
                invalidLines.insert(match.str());
            }
        }
        searchStart = match.suffix().first;
    }

    cleanedCode = removeInvalidDeclarations(cleanedCode, invalidLines);

    cleanedOutputFile << "\nCleaned Code:\n" << cleanedCode << "\n";
    cleanedOutputFile.close();
    outputFile.close();

    std::cout << "Analysis completed. Check 'output.txt' and 'output_cleaned.txt' for results.\n";

    return 0;
}
