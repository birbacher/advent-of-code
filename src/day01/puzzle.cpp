#include "day01/puzzle.hpp"

#include "iosupport/iosupport.hpp"

#include <algorithm>
#include <charconv>
#include <queue>
#include <iostream>

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

class StateB {
    static constexpr int topN = 3;

    /**
     * @brief priority queue configured with smallest elements a top()
     * Note that a comparator 'greater' will bring smallest elements to 'top'.
     */
    std::priority_queue<
            int,
            std::vector<int>,
            std::greater<>
        > currentMaximums;

    int currentGroupSum = 0;

    void pushToQueue(int x) {
        std::clog << "adding: " << x << '\n';
        currentMaximums.push(x);
        while (currentMaximums.size() > topN) {
            std::clog << "dropping: " << currentMaximums.top() << '\n';
            currentMaximums.pop();
        }
    }
public:
    void nextGroup() {
        pushToQueue(currentGroupSum);
        currentGroupSum = 0;
    }
    void valueInGroup(int value) {
        currentGroupSum += value;
    }
    /**
     * @brief Destructively compute the result from the current state.
     *
     * @return int The sum of 'topN' biggest groups.
     */
    int computeResult() && {
        int sum = 0;
        while(!currentMaximums.empty()) {
            std::clog << "found: " << currentMaximums.top() << '\n';
            sum += currentMaximums.top();
            currentMaximums.pop();
        }
        return sum;
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
    state.nextGroup();
    output << state.result() << '\n';
}

void puzzleB(std::istream& input, std::ostream& output)
{
    StateB state;
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
    state.nextGroup();
    output << std::move(state).computeResult() << '\n';
}

}
