#include <fstream>
#include <string>
#include <unordered_set>
#include <regex>
#include <iostream>
#include <sstream>
#include <vector>

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
        if (invalidLines.find(line) == invalidLines.end()) {
            result += line + "\n";
        }
    }
    return result;
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

std::string removeComments(const std::string& code) {
    std::string cleanedCode = code;
    std::regex singleLineCommentRegex(R"(//.*?$)");
    std::regex multiLineCommentRegex(R"(/\*[\s\S]*?\*/)");

    cleanedCode = std::regex_replace(cleanedCode, singleLineCommentRegex, "");
    cleanedCode = std::regex_replace(cleanedCode, multiLineCommentRegex, "");

    return cleanedCode;
}

std::string getTokenType(const std::string& token) {
    static const std::unordered_set<std::string> keywords = {
        "int", "float", "string", "void", "if", "else", "for", "while", "return"
    };

    if (token == "#") return "DIRECTIVE_SYMBOL";
    if (keywords.find(token) != keywords.end()) return "KEYWORD";
    if (token == "include") return "DIRECTIVE";
    if (token == "iostream") return "HEADER";
    if (token == "std") return "NAMESPACE";
    if (token == "cout" || token == "endl") return "STREAM";
    if (std::regex_match(token, std::regex(R"([{}();,])"))) return "DELIMITER";
    if (std::regex_match(token, std::regex(R"([-+*/=<>!&|])"))) return "OPERATOR";
    if (std::regex_match(token, std::regex(R"(\d+\.\d+|\d+)"))) return "NUMBER";
    if (std::regex_match(token, std::regex(R"(".*?")"))) return "STRING";
    if (std::regex_match(token, std::regex(R"([a-zA-Z_][a-zA-Z0-9_]*)"))) return "VARIABLE";

    return "UNKNOWN";
}

void analyzeTokens(const std::string& code, std::ofstream& outputFile) {
    std::regex tokenRegex(R"((#|include|int|float|string|void|if|else|for|while|return|[a-zA-Z_][a-zA-Z0-9_]*|[-+*/=<>!&|+=-=*=/=;{}(),]|\d+\.\d+|\d+|".*?"))");
    std::smatch match;
    std::istringstream stream(code);
    std::string line;
    int lineNumber = 0;

    outputFile << "\nLexical Units:\n";
    while (std::getline(stream, line)) {
        ++lineNumber;
        auto searchStart = line.cbegin();
        while (std::regex_search(searchStart, line.cend(), match, tokenRegex)) {
            std::string token = match.str();
            std::string tokenType = getTokenType(token);
            outputFile << "(" << tokenType << ", " << token << ", line " << lineNumber << ")\n";
            searchStart = match.suffix().first;
        }
    }
}

void analyzeGlobalVariables(const std::string& code, std::vector<Variable>& globalVariables, std::ofstream& outputFile) {
    std::regex globalVarRegex(R"((int|float|string)\s+([a-zA-Z_][a-zA-Z0-9_]*)\s*=\s*([^;]+);)");
    std::smatch match;
    std::istringstream stream(code);
    std::string line;
    bool insideFunction = false;

    while (std::getline(stream, line)) {
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

    outputFile << "Global Variables:\n";
    for (const auto& var : globalVariables) {
        outputFile << var.type << " " << var.name << " = " << var.value << "\n";
    }
}

void analyzeFunctions(const std::string& code, std::vector<Function>& functions, std::ofstream& outputFile) {
    std::regex functionRegex("(int|float|string|void)\\s+([a-zA-Z_][a-zA-Z0-9_]*)\\s*\\(([^)]*)\\)\\s*\\{");
    std::smatch match;
    std::string::const_iterator searchStart(code.cbegin());

    outputFile << "\nFunctions:\n";

    while (std::regex_search(searchStart, code.cend(), match, functionRegex)) {
        Function func;
        func.returnType = match[1];
        func.name = match[2];

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

        std::string functionBody = match.suffix().str();
        std::string functionStart = match[0].str();

        bool isRecursive = functionBody.find(func.name + "(") != std::string::npos;
        bool isIterative = functionBody.find("for") != std::string::npos || functionBody.find("while") != std::string::npos;

        functions.push_back(func);

        outputFile << func.returnType << " " << func.name << "(";
        for (size_t i = 0; i < func.parameters.size(); ++i) {
            outputFile << func.parameters[i].type << " " << func.parameters[i].name;
            if (i < func.parameters.size() - 1) outputFile << ", ";
        }
        outputFile << ") is ";
        if (isRecursive) {
            outputFile << "recursive\n";
        }
        else if (isIterative) {
            outputFile << "iterative\n";
        }
        else {
            outputFile << "neither iterative nor recursive\n";
        }

        searchStart = match.suffix().first;
    }
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

    analyzeGlobalVariables(cleanedCode, globalVariables, outputFile);

    analyzeFunctions(cleanedCode, functions, outputFile);

    analyzeTokens(cleanedCode, outputFile);

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
