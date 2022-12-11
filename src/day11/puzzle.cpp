#include "adventofcode.hpp"

#include "iosupport/iosupport.hpp"

#include <algorithm>
#include <array>
#include <cassert>
#include <charconv>
#include <functional>
#include <iostream>
#include <istream>
#include <iterator>
#include <limits>
#include <numeric>
#include <ostream>
#include <queue>
#include <unordered_set>

namespace advent::common {

namespace {

struct Item {
    int worryLevel{};
};

using WorryLevelChange = std::function<int(int)>;

struct Monkey {
    /// @brief The queue of items this monkey will look at.
    std::queue<Item> items;

    /// @brief The function to compute the new worry level when
    /// this monkey looks at an item.
    WorryLevelChange worryLevelChangeFn;

    /// @brief Number by which to divide to decide the destination
    /// for items from this monkey.
    int testDivisor{};

    int targetTrue{}, targetFalse{};

    int computeTargetForItem(Item const &item) const {
        return item.worryLevel % testDivisor == 0 ? targetTrue : targetFalse;
    }
};

std::function<int(int, int)> selectByOp(char op) {
    switch (op) {
    case '+':
        return std::plus<>{};
    case '-':
        return std::minus<>{};
    case '*':
        return std::multiplies<>{};
    case '/':
        return std::divides<>{};
    }
    using namespace std::literals;
    throw std::runtime_error("Unknown operation: "s + op);
}

WorryLevelChange selectWorryLevelChange(char op, std::string_view operand) {
    auto binOp = selectByOp(op);
    if (operand == "old") {
        return [=](int old) { return binOp(old, old); };
    } else {
        int constant;
        auto [ptr, ec] =
            std::from_chars(operand.begin(), operand.end(), constant);
        if (ec != std::errc())
            throw ec;

        return [=](int old) { return binOp(old, constant); };
    }
}

struct State {
    /// @brief The set of monkeys participating in the game.
    std::vector<Monkey> monkeys;

    /// @brief An array parallel to 'monkeys' to track their activity.
    std::vector<int> activities;
};

struct Interpreter {
    std::vector<Monkey> *const monkeys;
    std::istream &input;

    bool testString(std::string_view str) const {
        std::string tmp;
        input >> tmp;

        using namespace std::literals;
        if (input && tmp != str) {
            throw std::runtime_error(("Expected '"s += str) + "', but got: "s +=
                                     tmp);
        }
        return static_cast<bool>(input);
    }

    void expectString(std::string_view str) const {
        if (!testString(str)) {
            using namespace std::literals;
            throw std::runtime_error("Could not read expected input: "s += str);
        }
    }

    bool consumeChar(char expected) const {
        char actual;
        input >> actual;

        using namespace std::literals;
        if (!input) {
            throw std::runtime_error(
                "Could not read char from input, expected: "s + expected);
        }
        bool const matches = actual == expected;
        if (!matches) {
            input.unget();
        }
        return matches;
    }

    int readInt() const {
        int tmp;
        input >> tmp;

        if (!input) {
            throw std::runtime_error("Could not read int from input");
        }
        return tmp;
    }

    Monkey &readAndExpectIndex() {
        int index = readInt();
        if (index != monkeys->size()) {
            throw std::runtime_error("Read in invalid index for monkey");
        }
        expectString(":");
        return monkeys->emplace_back();
    }

    void readStartingItems(std::queue<Item> &items) {
        expectString("Starting");
        expectString("items:");
        do {
            Item item{readInt()};
            items.push(item);
        } while (consumeChar(','));
    }

    void readOperation(WorryLevelChange &fn) {
        expectString("Operation:");
        expectString("new");
        expectString("=");
        expectString("old");
        char op;
        std::string operand;
        input >> op >> operand;
        fn = selectWorryLevelChange(op, operand);
    }

    void readTestCondition(int &divisor) {
        expectString("Test:");
        expectString("divisible");
        expectString("by");
        input >> divisor;
        if (!input) {
            throw std::runtime_error(
                "Cannot interpret test condition as integer");
        }
    }

    void readTarget(std::string_view whichCase, int &target) {
        expectString("If");
        expectString(whichCase);
        expectString("throw");
        expectString("to");
        expectString("monkey");
        input >> target;
        if (!input) {
            throw std::runtime_error("Cannot interpret target as integer");
        }
    }

    void readMonkey() {
        if (!testString("Monkey"))
            return;
        Monkey &result = readAndExpectIndex();
        readStartingItems(result.items);
        readOperation(result.worryLevelChangeFn);
        readTestCondition(result.testDivisor);
        readTarget("true:", result.targetTrue);
        readTarget("false:", result.targetFalse);
    }

    void readAll() {
        while (input) {
            readMonkey();
        }
    }
};

void executeItem(State &state, int monkeyIndex) {
    auto &monkey = state.monkeys.at(monkeyIndex);
    assert(!monkey.items.empty());

    Item current = monkey.items.front();
    monkey.items.pop();
    current.worryLevel = monkey.worryLevelChangeFn(current.worryLevel);
    current.worryLevel /= 3;
    int const targetIndex = monkey.computeTargetForItem(current);
    state.monkeys.at(targetIndex).items.push(current);
    ++state.activities.at(monkeyIndex);
}

void executeTurn(State &state, int monkeyIndex) {
    while (!state.monkeys.at(monkeyIndex).items.empty()) {
        executeItem(state, monkeyIndex);
    }
}

void executeRound(State &state) {
    for (int monkeyIndex = 0; monkeyIndex < state.monkeys.size();
         ++monkeyIndex) {
        executeTurn(state, monkeyIndex);
    }
}

} // namespace

template <> void puzzleA<2022, 11>(std::istream &input, std::ostream &output) {
    State state;
    Interpreter interpreter{&state.monkeys, input};
    interpreter.readAll();
    state.activities.resize(state.monkeys.size());
    for (int i = 0; i < 20; ++i) {
        executeRound(state);
    }
    std::sort(state.activities.begin(), state.activities.end(),
              std::greater<>{});
    output << state.activities.at(0) * state.activities.at(1) << '\n';
}

template <> void puzzleB<2022, 11>(std::istream &input, std::ostream &output) {}

} // namespace advent::common
