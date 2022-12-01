#include "day01/puzzle.hpp"

#include <stdexcept>
#include <iostream>
#include <fstream>

int main(int argc, const char* argv[])
{
    std::ifstream file;

    std::istream *input = &std::cin;
    if (argc > 1) {
        file.open(argv[1]);
        if (!file) throw std::runtime_error("Could not open input file");
        input = &file;
    }

    advent::day01::puzzleA(*input, std::cout);
    std::cout << '\n';
}
