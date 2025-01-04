#ifndef SEMANTIC_ANALYZER_H
#define SEMANTIC_ANALYZER_H

#include "lexer.h"
#include <vector>
#include <string>
#include <unordered_map>

class SemanticAnalyzer {
public:
    SemanticAnalyzer(const std::vector<GlobalVariable>& globals, const std::vector<FunctionInfo>& functions);
    void analyze();
    const std::vector<std::string>& getErrors() const;
    void printErrors() const;

private:
    const std::vector<GlobalVariable>& globals;
    const std::vector<FunctionInfo>& functions;
    std::vector<std::string> errors;

    void checkVariableUsage();
    void checkFunctionCalls();
};

#endif // SEMANTIC_ANALYZER_H
