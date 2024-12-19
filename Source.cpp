#include <iostream>
#include <fstream>
#include "lexer.h"

int main() {
    // Citește codul sursă dintr-un fișier
    std::ifstream inputFile("in.txt");

    if (!inputFile.is_open()) {
        std::cerr << "Nu s-a putut deschide fișierul!" << std::endl;
        return 1;
    }

    std::string code((std::istreambuf_iterator<char>(inputFile)), std::istreambuf_iterator<char>());
    inputFile.close();

    // Creează un obiect Lexer și tokenizează codul citit
    Lexer lexer(code);
    lexer.tokenize();
    lexer.printTokens();

    return 0;
}
