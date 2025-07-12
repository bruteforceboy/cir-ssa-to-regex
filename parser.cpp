#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include "ssa_to_regex.h"

int main(int argc, char *argv[]) {
    const char *inputFile = (argc > 1 ? argv[1] : "in.txt");
    const char *outputFile = (argc > 2 ? argv[2] : "out.txt");

    std::ifstream infile(inputFile);
    if (!infile.is_open()) {
        std::cerr << "Error: Unable to open input file '" << inputFile
                  << "'.\n";
        return 1;
    }

    std::vector<std::string> prog;
    std::string line;
    while (std::getline(infile, line))
        prog.push_back(line);
    infile.close();

    ssaToRegex(prog);

    std::ofstream outfile(outputFile);
    if (!outfile.is_open()) {
        std::cerr << "Error: Unable to open output file '" << outputFile
                  << "'.\n";
        return 1;
    }

    for (size_t i = 0; i < prog.size(); ++i) {
        outfile << prog[i];
        if (i + 1 < prog.size())
            outfile << '\n';
    }
    outfile.close();

    return 0;
}
