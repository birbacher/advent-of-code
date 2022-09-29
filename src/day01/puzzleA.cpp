#include "day01/puzzleA.hpp"

#include "iosupport/iosupport.hpp"

#include <charconv>
#include <optional>

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

}
