#include <fstream>
#include <string>
#include <unordered_set>
#include <regex>
#include <iostream>
#include <sstream>
#include <vector>

int code = 10;

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

void analyzeFunctions(const std::string& code, std::vector<Function>& functions, std::ofstream& outputFile, std::ofstream& errorsFile) {
    std::regex functionRegex(R"((int|float|string|void)\s+([a-zA-Z_][a-zA-Z0-9_]*)\s*\(([^)]*)\)\s*\{)");
    std::regex localVarRegex(R"((int|float|string)\s+([a-zA-Z_][a-zA-Z0-9_]*)\s*=\s*([^;]+);)");
    std::regex controlStructureRegex(R"((if|else if|else|for|while))");
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
                std::cerr << "Debug: Incomplete function detected - " << func.name << "\n";
                currentCode.clear();
                continue;
            }

            if (func.returnType != "void" && functionBody.find("return") == std::string::npos) {
                errorsFile << "Error: Non-void function '" << func.name
                    << "' does not have a return statement.\n";
            }


            std::smatch localVarMatch;
            std::string::const_iterator localSearchStart(functionBody.cbegin());
            while (std::regex_search(localSearchStart, functionBody.cend(), localVarMatch, localVarRegex)) {
                func.localVariables.push_back({ localVarMatch[1], localVarMatch[2], localVarMatch[3], func.line });
                localSearchStart = localVarMatch.suffix().first;
            }

            std::smatch controlMatch;
            std::string::const_iterator controlSearchStart(functionBody.cbegin());
            std::vector<std::pair<std::string, int>> controlStructures;
            while (std::regex_search(controlSearchStart, functionBody.cend(), controlMatch, controlStructureRegex)) {
                int structureLine = func.line;
                for (auto it = functionBody.cbegin(); it != controlMatch[0].first; ++it) {
                    if (*it == '\n') {
                        structureLine++;
                    }
                }
                controlStructures.emplace_back(controlMatch.str(), structureLine);
                controlSearchStart = controlMatch.suffix().first;
            }

            std::regex recursiveCallRegex("\\b" + func.name + "\\s*\\(");
            bool isRecursive = std::regex_search(functionBody, recursiveCallRegex);
            bool isIterative = functionBody.find("for") != std::string::npos || functionBody.find("while") != std::string::npos;

            outputFile << func.returnType << " " << func.name << "(";
            for (size_t i = 0; i < func.parameters.size(); ++i) {
                outputFile << func.parameters[i].type << " " << func.parameters[i].name;
                if (i < func.parameters.size() - 1) outputFile << ", ";
            }
            outputFile << ") is ";
			if (func.name == "main") {
				outputFile << "the main function";
			}
			else if (isRecursive) {
                outputFile << "recursive";
            }
            else if (isIterative) {
                outputFile << "iterative";
            }
            else {
                outputFile << "neither iterative nor recursive";
            }
            outputFile << "\n";

            if (!func.localVariables.empty()) {
                outputFile << "Local Variables:\n";
                for (const auto& var : func.localVariables) {
                    outputFile << "- " << var.type << " " << var.name << " = " << var.value << "\n";
                }
            }

            if (!controlStructures.empty()) {
                outputFile << "Control Structures:\n";
                for (const auto& [control, structureLine] : controlStructures) {
                    outputFile << "- " << control << ", line " << structureLine << "\n";
                }
            }

            functions.push_back(func);
            currentCode.clear(); 

            outputFile << "\n";
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
