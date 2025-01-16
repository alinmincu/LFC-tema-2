#include <fstream>
#include <string>
#include <unordered_set>
#include <regex>
#include <iostream>
#include <sstream>
#include <vector>
#include <unordered_map>

int code = 10;
std::unordered_map<std::string, std::string> functionReturnTypes;

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
    std::regex intRegex("^[-+]?\\d+$");
    std::regex floatRegex("^[-+]?\\d*\\.\\d+$");
    std::regex variableRegex(R"([a-zA-Z_][a-zA-Z0-9_]*)"); 
    std::regex expressionRegex(
        R"([a-zA-Z_][a-zA-Z0-9_]*(\s*[\+\-\*/]\s*[a-zA-Z_][a-zA-Z0-9_]*|\s*[\+\-\*/]\s*\d+(\.\d+)?)*|(\b[a-zA-Z_][a-zA-Z0-9_]*\s*\(.*\)))");
    std::regex functionCallRegex(R"([a-zA-Z_][a-zA-Z0-9_]*\s*\(.*\))");

    if (std::regex_match(value, functionCallRegex)) {
        std::string functionName = value.substr(0, value.find('(')); // Extragem numele funcției
        if (functionReturnTypes.find(functionName) != functionReturnTypes.end()) {
            std::string returnType = functionReturnTypes[functionName];
            if (type == "int" && (returnType == "float" || returnType == "double")) {
                return false; 
            }
            return true; 
        }
    }
    if (type == "int") {
        if (std::regex_match(value, floatRegex)) {
            return false; 
        }
        return std::regex_match(value, intRegex) || std::regex_match(value, expressionRegex);
    }
    if (type == "float" || type == "double") {
        return std::regex_match(value, floatRegex) || std::regex_match(value, intRegex) || std::regex_match(value, expressionRegex);
    }
    if (type == "string") {
        return value.front() == '"' && value.back() == '"';
    }
    return false; 
}

bool areTypesCompatible(const std::string& type1, const std::string& type2) {
    std::unordered_set<std::string> numericTypes = { "int", "float", "double" };

    if (numericTypes.find(type1) != numericTypes.end() && numericTypes.find(type2) != numericTypes.end()) {
        return true;
    }
    return type1 == type2;
}

std::string getExpressionType(const std::string& expression, const Function& func) {
    std::regex variableRegex(R"([a-zA-Z_][a-zA-Z0-9_]*)");
    std::regex numberRegex(R"(\d+)");
    std::regex floatNumberRegex(R"(\d+\.\d+)");
    std::regex stringRegex(R"(".*?")");
    std::regex operationRegex(R"(([a-zA-Z_][a-zA-Z0-9_]*|\d+(\.\d+)?)\s*[\+\-\*/]\s*([a-zA-Z_][a-zA-Z0-9_]*|\d+(\.\d+)?))");

    if (std::regex_match(expression, stringRegex)) return "string";
    if (std::regex_match(expression, floatNumberRegex)) return "float";
    if (std::regex_match(expression, numberRegex)) return "int";

    if (std::regex_match(expression, operationRegex)) {
        std::smatch operationMatch;
        std::regex_search(expression, operationMatch, operationRegex);

        std::string operand1 = operationMatch[1];
        std::string operand2 = operationMatch[3];

        std::string type1 = getExpressionType(operand1, func);
        std::string type2 = getExpressionType(operand2, func);

        if (!areTypesCompatible(type1, type2)) return "unknown";

        if (type1 == "float" || type2 == "float") return "float";
        if (type1 == "double" || type2 == "double") return "double";
        return "int";
    }

    for (const auto& param : func.parameters) {
        if (param.name == expression) return param.type;
    }
    for (const auto& localVar : func.localVariables) {
        if (localVar.name == expression) return localVar.type;
    }

    return "unknown";
}

std::string removeComments(const std::string& code) {
    std::string cleanedCode = code;
    std::regex singleLineCommentRegex(R"(//.*?$)");
    std::regex multiLineCommentRegex(R"(/\*[\s\S]*?\*/)");

    cleanedCode = std::regex_replace(cleanedCode, singleLineCommentRegex, "");
    cleanedCode = std::regex_replace(cleanedCode, multiLineCommentRegex, "");

    return cleanedCode;
}

std::string getTokenType(const std::string& token, const std::string& nextToken = "") {
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

    if (!nextToken.empty() && nextToken == "(") {
        return "FUNCTION_NAME";
    }

    if (std::regex_match(token, std::regex(R"([a-zA-Z_][a-zA-Z0-9_]*)"))) return "VARIABLE";

    return "UNKNOWN";
}

void analyzeTokens(const std::string& code, std::ofstream& outputFile) {
    std::regex tokenRegex(R"((#|include|int|float|string|void|if|else|for|while|return|[a-zA-Z_][a-zA-Z0-9_]*|[-+*/=<>!&|+=-=*=/=;{}(),]|\d+\.\d+|\d+|".*?"))");
    std::smatch match;
    std::istringstream stream(code);
    std::string line;
    int lineNumber = 0;

    outputFile << "Lexical Units:\n";
    while (std::getline(stream, line)) {
        ++lineNumber;
        auto searchStart = line.cbegin();
        std::string nextToken;

        while (std::regex_search(searchStart, line.cend(), match, tokenRegex)) {
            std::string token = match.str();

            auto nextSearchStart = match.suffix().first;
            if (std::regex_search(nextSearchStart, line.cend(), match, tokenRegex)) {
                nextToken = match.str();
            }
            else {
                nextToken.clear();
            }

            std::string tokenType = getTokenType(token, nextToken);
            outputFile << "(" << tokenType << ", " << token << ", line " << lineNumber << ")\n";
            searchStart = nextSearchStart;
        }
    }
}

void analyzeGlobalVariables(const std::string& code, std::vector<Variable>& globalVariables, std::ofstream& outputFile) {
    std::regex globalVarRegex(R"((int|float|double|string)\s+([a-zA-Z_][a-zA-Z0-9_]*)(\s*=\s*([^;]+))?;)");
    std::smatch match;
    std::istringstream stream(code);
    std::string line;

    int braceBalance = 0; 

    while (std::getline(stream, line)) {
        braceBalance += std::count(line.begin(), line.end(), '{') - std::count(line.begin(), line.end(), '}');

        if (braceBalance == 0 && std::regex_search(line, match, globalVarRegex)) {
            std::string type = match[1];
            std::string name = match[2];
            std::string value = match[4];
            globalVariables.push_back({ type, name, value, 0 });
        }
    }

    outputFile << "Global Variables:\n";
    for (const auto& var : globalVariables) {
        outputFile << var.type << " " << var.name;
        if (!var.value.empty()) {
            outputFile << " = " << var.value;
        }
        outputFile << "\n";
    }
}

void analyzeFunctions(const std::string& code, std::vector<Function>& functions, std::ofstream& outputFile, std::ofstream& errorsFile) {
    std::regex functionRegex(R"((int|float|string|void)\s+([a-zA-Z_][a-zA-Z0-9_]*)\s*\(([^)]*)\)\s*\{)");
    std::regex localVarRegex(R"((int|float|string)\s+([a-zA-Z_][a-zA-Z0-9_]*)\s*=\s*([^;]+);)");
    std::regex operationRegex(R"(([a-zA-Z_][a-zA-Z0-9_]*)\s*[\+\-\*/]\s*([a-zA-Z_][a-zA-Z0-9_]*))");
    std::regex returnRegex(R"(\breturn\s+(.*?);)");
    std::smatch match;
    std::istringstream stream(code);
    std::string line;
    std::string currentCode;
    int lineCounter = 0;

    while (std::getline(stream, line)) {
        lineCounter++;
        currentCode += line + "\n";

        if (std::regex_search(currentCode, match, functionRegex)) {
            Function func;
            func.returnType = match[1];
            func.name = match[2];
            func.line = lineCounter;

            functionReturnTypes[func.name] = func.returnType;

            std::string params = match[3];
            std::regex paramRegex(R"((int|float|string)\s+([a-zA-Z_][a-zA-Z0-9_]*))");
            std::smatch paramMatch;
            std::string::const_iterator searchStart(params.cbegin());

            while (std::regex_search(searchStart, params.cend(), paramMatch, paramRegex)) {
                func.parameters.push_back({ paramMatch[1], paramMatch[2], "", func.line });
                searchStart = paramMatch.suffix().first;
            }

            std::unordered_set<std::string> parameterNames;
            for (const auto& param : func.parameters) {
                if (parameterNames.find(param.name) != parameterNames.end()) {
                    errorsFile << "Error: Parameter '" << param.name
                        << "' is already declared in function '" << func.name << "'.\n";
                }
                else {
                    parameterNames.insert(param.name);
                }
            }

            std::string functionBody;
            int braceCount = 1;
            size_t startPos = currentCode.find('{', match.position());

            functionBody = currentCode.substr(startPos + 1);

            while (braceCount > 0 && std::getline(stream, line)) {
                lineCounter++;
                for (char ch : line) {
                    if (ch == '{') braceCount++;
                    if (ch == '}') braceCount--;
                }
                functionBody += line + "\n";
            }

            if (braceCount != 0) {
                errorsFile << "Error: Incomplete function body for '" << func.name << "'.\n";
                currentCode.clear();
                continue;
            }

            std::smatch operationMatch;
            std::string::const_iterator operationSearchStart(functionBody.cbegin());
            while (std::regex_search(operationSearchStart, functionBody.cend(), operationMatch, operationRegex)) {
                std::string var1 = operationMatch[1];
                std::string var2 = operationMatch[2];

                std::string type1, type2;
                for (const auto& param : func.parameters) {
                    if (param.name == var1) type1 = param.type;
                    if (param.name == var2) type2 = param.type;
                }
                for (const auto& localVar : func.localVariables) {
                    if (localVar.name == var1) type1 = localVar.type;
                    if (localVar.name == var2) type2 = localVar.type;
                }

                if (!type1.empty() && !type2.empty() && !areTypesCompatible(type1, type2)) {
                    errorsFile << "Error: Incompatible types in operation between '" << var1 << "' (" << type1
                        << ") and '" << var2 << "' (" << type2 << ") in function '" << func.name << "'.\n";
                }
                operationSearchStart = operationMatch.suffix().first;
            }

            if (func.returnType != "void") {
                std::smatch returnMatch;
                std::string::const_iterator returnSearchStart(functionBody.cbegin());
                while (std::regex_search(returnSearchStart, functionBody.cend(), returnMatch, returnRegex)) {
                    std::string returnExpression = returnMatch[1];
                    std::string returnType = getExpressionType(returnExpression, func);

                    if ((func.returnType == "float" && returnType == "int") ||
                        (func.returnType == "double" && (returnType == "int" || returnType == "float"))) {
                        returnSearchStart = returnMatch.suffix().first;
                        continue;
                    }

                    if (returnType != func.returnType) {
                        errorsFile << "Error: Return type mismatch in function '" << func.name << "'. Expected '"
                            << func.returnType << "', but got '" << returnType << "' for expression '"
                            << returnExpression << "'.\n";
                    }
                    returnSearchStart = returnMatch.suffix().first;
                }
            }

            functions.push_back(func);
            currentCode.clear();
        }
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

    std::ofstream lexicalFile("lexical_units.txt");
    std::ofstream globalVarFile("global_variables.txt");
    std::ofstream functionsFile("functions.txt");
    std::ofstream errorsFile("errors.txt");
    std::ofstream outputFile("output.txt");

    if (!lexicalFile || !globalVarFile || !functionsFile || !errorsFile || !outputFile) {
        std::cerr << "Error: Could not open output files.\n";
        return 1;
    }

    std::unordered_set<std::string> declaredGlobalVariables;
    std::vector<Variable> globalVariables;
    std::vector<Function> functions;

    std::string cleanedCode = removeComments(code);

    outputFile << "Cleaned Code:\n" << cleanedCode;

    if (cleanedCode.find("int main(") == std::string::npos) {
        errorsFile << "Error: Function main() not found.\n";
    }

    analyzeTokens(cleanedCode, lexicalFile);

    analyzeGlobalVariables(cleanedCode, globalVariables, globalVarFile);

    for (const auto& var : globalVariables) {
        if (declaredGlobalVariables.find(var.name) != declaredGlobalVariables.end()) {
            errorsFile << "Error: Global variable '" << var.name << "' is already declared.\n";
        }
        else if (!isInitializationValid(var.type, var.value)) {
            errorsFile << "Error: Invalid initialization for global variable '" << var.name
                << "' with value '" << var.value << "'.\n";
        }
        else {
            declaredGlobalVariables.insert(var.name);
        }
    }

    analyzeFunctions(cleanedCode, functions, functionsFile, errorsFile);

    for (const auto& func : functions) {
        std::unordered_set<std::string> declaredLocalVariables;
        for (const auto& localVar : func.localVariables) {
            if (declaredLocalVariables.find(localVar.name) != declaredLocalVariables.end()) {
                errorsFile << "Error: Variable '" << localVar.name << "' is already declared locally in function '"
                    << func.name << "'.\n";
            }
            else if (!isInitializationValid(localVar.type, localVar.value)) {
                errorsFile << "Error: Invalid initialization for local variable '" << localVar.name
                    << "' in function '" << func.name << "' with value '" << localVar.value << "'.\n";
            }
            else {
                declaredLocalVariables.insert(localVar.name);
            }
        }
    }

    lexicalFile.close();
    globalVarFile.close();
    functionsFile.close();
    errorsFile.close();
    outputFile.close();

    std::cout << "Analysis completed. Check the generated files for results.\n";
    return 0;
}
