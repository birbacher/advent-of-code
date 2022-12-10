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
#include <map>
#include <numeric>
#include <ostream>
#include <unordered_set>

namespace advent::common {

namespace {

struct Effect {
    int cycles{};
    int deltaX{};
};

std::istream &operator>>(std::istream &stream, Effect &effect) {
    std::string opcode;
    if (stream >> opcode) {
        if (opcode == "noop") {
            effect = Effect{1, 0};
        } else if (opcode == "addx") {
            effect.cycles = 2;
            stream >> effect.deltaX;
        } else {
            throw std::runtime_error("Unknown instruction for Effect: " +
                                     opcode);
        }
    }
    return stream;
}

struct CpuState {
    int cycle{1}; // the number of the current cycle
    int x{1};     // the register x
};

struct SignalObserver {
    CpuState cpu;
    int sumSignals{};
};

constexpr int firstCycleThatCounts = 20;
constexpr int importantCyclePeriod = 40;
bool isImportantCycle(int cycle) {
    return cycle % importantCyclePeriod == firstCycleThatCounts;
}

void applyEffect(SignalObserver &ob, Effect effect) {
    while (effect.cycles--) {
        if (isImportantCycle(ob.cpu.cycle)) {
            ob.sumSignals += ob.cpu.cycle * ob.cpu.x;
        }
        ++ob.cpu.cycle;
    }
    ob.cpu.x += effect.deltaX;
}

} // namespace

template <> void puzzleA<2022, 10>(std::istream &input, std::ostream &output) {
    SignalObserver ob;
    std::for_each(std::istream_iterator<Effect>(input),
                  std::istream_iterator<Effect>(),
                  std::bind(&applyEffect, std::ref(ob), std::placeholders::_1));
    output << ob.sumSignals << '\n';
}

template <> void puzzleB<2022, 10>(std::istream &input, std::ostream &output) {}

} // namespace advent::common
