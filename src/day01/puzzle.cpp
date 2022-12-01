#include "day01/puzzle.hpp"

#include "iosupport/iosupport.hpp"

#include <algorithm>
#include <charconv>

namespace advent::day01 {

namespace {

class StateA {
    int currentMax = 0;
    int currentGroupSum = 0;
public:
    void nextGroup() {
        currentMax = std::max(currentMax, currentGroupSum);
        currentGroupSum = 0;
    }
    void valueInGroup(int value) {
        currentGroupSum += value;
    }
    int result() const {
        return currentMax;
    }
};

}

void puzzleA(std::istream& input, std::ostream& output)
{
    StateA state;
    iosupport::loadFileInLines(input, [&](std::string_view sv) {
        if (sv.empty()) {
            state.nextGroup();
        }
        else {
            int number;
            auto [ptr, ec] = std::from_chars(sv.begin(), sv.end(), number);
            if (ec != std::errc()) throw ec;

            state.valueInGroup(number);
        }
    });
    output << state.result() << '\n';
}

void puzzleB(std::istream& input, std::ostream& output)
{
}

}
