#include "SemanticAnalyzer.h"

SemanticAnalyzer::SemanticAnalyzer(const std::vector<GlobalVariable>& globals, const std::vector<FunctionInfo>& functions)
    : globals(globals), functions(functions) {
}

void SemanticAnalyzer::analyze() {
    for (const auto& func : functions) {
        if (func.name.empty()) {
            errors.push_back("Function name missing.");
        }
        if (func.isRecursive) {
            std::cout << "Recursive function detected: " << func.name << "\n";
        }
    }
}

void SemanticAnalyzer::checkVariableUsage() {
    // Verifică dacă variabilele sunt definite înainte de utilizare
    for (const auto& global : globals) {
        if (global.name.empty() || global.type.empty()) {
            errors.push_back("Global variable has an incomplete declaration: " + global.name);
        }
    }
}

void SemanticAnalyzer::checkFunctionCalls() {
    // Verifică apelurile de funcții și recursivitatea
    for (const auto& func : functions) {
        if (func.isRecursive && func.name.empty()) {
            errors.push_back("Recursive function has no proper name.");
        }
    }
}

const std::vector<std::string>& SemanticAnalyzer::getErrors() const {
    return errors;
}

void SemanticAnalyzer::printErrors() const {
    std::cout << "Semantic Errors:\n";
    for (const auto& error : errors) {
        std::cout << error << "\n";
    }
}
