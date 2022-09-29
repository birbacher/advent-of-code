#include "day02/puzzle.hpp"

#include <charconv>
#include <deque>
#include <istream>
#include <optional>
#include <stdexcept>
#include <string>

#include "iosupport/iosupport.hpp"

namespace advent::day02 {

template <typename Strategy>
void interpretInput(std::istream &input, Strategy &strategy) {
    std::string command;
    int amount;

    while (input >> command >> amount) {
        if (command == "forward") {
            strategy.forward(amount);
        } else if (command == "down") {
            strategy.down(amount);
        } else if (command == "up") {
            strategy.up(amount);
        } else {
            throw std::runtime_error("Unknown command: " + command);
        }
    }
}

void puzzleA(std::istream &input, std::ostream &output) {
    struct {
        int horizontal = 0, depth = 0;
        void forward(int amount) { horizontal += amount; }
        void down(int amount) { depth += amount; }
        void up(int amount) { depth -= amount; }
        int result() const { return horizontal * depth; }
    } strategy;
    interpretInput(input, strategy);
    output << strategy.result();
}

void puzzleB(std::istream &input, std::ostream &output) {
    struct {
        int horizontal = 0, depth = 0, aim = 0;
        void forward(int amount) {
            horizontal += amount;
            depth += aim * amount;
        }
        void down(int amount) { aim += amount; }
        void up(int amount) { aim -= amount; }
        int result() const { return horizontal * depth; }
    } strategy;
    interpretInput(input, strategy);
    output << strategy.result();
}

} // namespace advent::day02
