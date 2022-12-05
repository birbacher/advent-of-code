#include "adventofcode.hpp"

#include "iosupport/iosupport.hpp"

#include <algorithm>
#include <cassert>
#include <functional>
#include <istream>
#include <iterator>
#include <numeric>
#include <ostream>
#include <stack>
#include <stdexcept>
#include <string>
#include <vector>

namespace advent::common {

namespace {

struct Create {
    char id{};
};

using Stack = std::stack<Create>;

struct Action {
    std::size_t amount{};
    std::size_t sourceStack{};
    std::size_t destStack{};
};

class Floor {
    std::vector<Stack> stacks;

  public:
    Floor(std::size_t size) : stacks(size) {}

    Stack &at(std::size_t oneBasedIndex) {
        return stacks.at(oneBasedIndex - 1);
    }

    void perform(Action const &action) {
        Stack &src = at(action.sourceStack);
        Stack &dst = at(action.destStack);
        if (src.size() < action.amount) {
            throw std::runtime_error(
                "Too few items on source stack to perform action");
        }
        for (std::size_t i = 0; i < action.amount; ++i) {
            dst.push(src.top());
            src.pop();
        }
    }

    void printTops(std::ostream &stream) const {
        std::transform(stacks.begin(), stacks.end(),
                       std::ostream_iterator<char>(stream),
                       [](Stack const &stack) {
                           if (stack.empty()) {
                               throw std::runtime_error(
                                   "One stack is empty during output");
                           }
                           return stack.top().id;
                       });
    }
};

std::vector<std::string> readFloorStackLines(std::istream &stream) {
    std::vector<std::string> lines;
    for (;;) {
        if (!std::getline(stream, lines.emplace_back())) {
            throw std::runtime_error(
                "Input doesn't contain line with stack numbers");
        }

        if (lines.back().find('[') == std::string::npos) {
            // Break on first line that doesn't contain a '['. This must be the
            // numbers line.
            break;
        }
    }
    return lines;
}

void interpretStacks(Floor &floor, std::vector<std::string> const &lines) {
    std::for_each(lines.rbegin(), lines.rend(), [&](std::string_view line) {
        for (std::size_t charPos = 1, idx = 1; charPos < line.size();
             charPos += 4, ++idx) {
            const char c = line.at(charPos);
            if (std::isalpha(c)) {
                floor.at(idx).push(Create{c});
            }
        }
    });
}

Floor readFloor(std::istream &stream) {
    auto lines = readFloorStackLines(stream);
    assert(!lines.empty());

    Floor floor((lines.back().size() + 1) / 4);
    lines.pop_back();

    interpretStacks(floor, lines);
    return floor;
}

std::istream &operator>>(std::istream &stream, Action &action) {
    std::string tmp;
    stream >> tmp;
    if (tmp != "move") {
        stream.setstate(stream.rdstate() | std::ios_base::failbit);
        return stream;
    }

    stream >> action.amount;

    stream >> tmp;
    if (tmp != "from") {
        stream.setstate(stream.rdstate() | std::ios_base::failbit);
        return stream;
    }

    stream >> action.sourceStack;

    stream >> tmp;
    if (tmp != "to") {
        stream.setstate(stream.rdstate() | std::ios_base::failbit);
        return stream;
    }

    stream >> action.destStack;
    return stream;
}

} // namespace

template <> void puzzleA<2022, 5>(std::istream &input, std::ostream &output) {
    Floor floor = readFloor(input);
    std::for_each(
        std::istream_iterator<Action>(input), std::istream_iterator<Action>(),
        std::bind(&Floor::perform, &(floor), std::placeholders::_1));
    floor.printTops(output);
}

template <> void puzzleB<2022, 5>(std::istream &input, std::ostream &output) {}

} // namespace advent::common
