#include "day02/puzzle.hpp"

#include "iosupport/iosupport.hpp"

#include <charconv>
#include <optional>
#include <deque>
#include <istream>
#include <string>
#include <stdexcept>

namespace advent::day02 {

void puzzleA(std::istream& input, std::ostream& output)
{
    std::string command;
    int amount;

    int horizontal = 0, depth = 0;
    while (input >> command >> amount) {
        if (command == "forward") {
            horizontal += amount;
        }
        else if (command == "down") {
            depth += amount;
        }
        else if (command == "up") {
            depth -= amount;
        }
        else {
            throw std::runtime_error("Unknown command: " + command);
        }
    }
    output << horizontal * depth;
}

void puzzleB(std::istream& input, std::ostream& output)
{
    std::string command;
    int amount;

    int horizontal = 0, depth = 0, aim = 0;
    while (input >> command >> amount) {
        if (command == "forward") {
            horizontal += amount;
            depth += aim * amount;
        }
        else if (command == "down") {
            aim += amount;
        }
        else if (command == "up") {
            aim -= amount;
        }
        else {
            throw std::runtime_error("Unknown command: " + command);
        }
    }
    output << horizontal * depth;
}

}
