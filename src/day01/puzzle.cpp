#include "day01/puzzle.hpp"

#include "iosupport/iosupport.hpp"

#include <charconv>
#include <optional>
#include <deque>

namespace advent::day01 {

void puzzleA(std::istream& input, std::ostream& output)
{
    std::optional<int> last;
    int increments = 0;
    iosupport::loadFileInLines(input, [&](std::string_view sv) {
        int number;
        auto [ptr, ec] = std::from_chars(sv.begin(), sv.end(), number);
        if (ec != std::errc()) throw ec;

        if (last) {
            if (*last < number) ++increments;
        }
        last = number;
    });
    output << increments;
}

void puzzleB(std::istream& input, std::ostream& output)
{
    std::deque<int> window;
    int currentSum = 0;
    int increments = 0;
    iosupport::loadFileInLines(input, [&](std::string_view sv) {
        int number;
        auto [ptr, ec] = std::from_chars(sv.begin(), sv.end(), number);
        if (ec != std::errc()) throw ec;

        const int oldSum = currentSum;

        window.push_back(number);
        currentSum += number;

        if (window.size() > 3) {
            currentSum -= window.front();
            window.pop_front();

            if (oldSum < currentSum) ++increments;
        }
    });
    output << increments;
}

}
