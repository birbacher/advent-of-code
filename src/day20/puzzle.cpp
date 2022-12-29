#include "adventofcode.hpp"

#include <algorithm>
#include <array>
#include <cassert>
#include <charconv>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <istream>
#include <iterator>
#include <numeric>
#include <ostream>
#include <string>
#include <tuple>
#include <vector>

namespace advent::common {

namespace {

struct State {
    const std::vector<int> numbers;
    const int indexZero{};
    std::vector<int> numToPos;
    std::vector<int> posToOrigin;

    State(std::istream &stream)
        : numbers(std::istream_iterator<int>{stream},
                  std::istream_iterator<int>{}),
          indexZero(std::distance(
              numbers.begin(), std::find(numbers.begin(), numbers.end(), 0))),
          numToPos(numbers.size()) {
        std::iota(numToPos.begin(), numToPos.end(), 0);
        posToOrigin = numToPos;

        if (indexZero == numbers.size()) {
            throw std::runtime_error("No zero in input");
        }
    }

    auto add(int delta) {
        return [this, delta](int origin) {
            auto &p = numToPos.at(origin);
            p = mod(p + delta);
        };
    }
    auto sub(int delta) { return add(-delta); }

    int mod(int value) {
        int N = static_cast<int>(numToPos.size());
        auto res = value % N;
        return res >= 0 ? res : res + N;
    }
    int mod1(int value) {
        int N = static_cast<int>(numToPos.size()) - 1;
        auto res = value % N;
        return res >= 0 ? res : res + N;
    }

    void rotInRange(int src, int mid, int lst) {
        auto const b = posToOrigin.begin();
        auto const s = b + src;
        auto const m = b + mid;
        auto const l = b + lst;
        std::for_each(s, m, add(l - m));
        std::for_each(m, l, sub(m - s));
        std::rotate(s, m, l);
        assertRelation();
    }

    void processOriginalIndex(int i) {
        auto n = numbers.at(i);
        auto fromPos = numToPos.at(i);
        auto toPos = mod1(fromPos + n);
        const bool actuallyGoingRight = toPos > fromPos;
        if (actuallyGoingRight) {
            rotInRange(fromPos, fromPos + 1, toPos + 1);
        } else {
            rotInRange(toPos, fromPos, fromPos + 1);
        }
    }

    void mix() {
        // print(std::clog);
        for (int i = 0; i < numbers.size(); ++i) {
            processOriginalIndex(i);
            // print(std::clog);
        }
    }

    int sum123k() {
        const int posZero = numToPos.at(indexZero);
        int sum = 0;
        for (int offset : {1000, 2000, 3000}) {
            const int p = mod(posZero + offset);
            sum += numbers.at(posToOrigin.at(p));
        }
        return sum;
    }

    void assertRelation() {
        assert(numToPos.size() == posToOrigin.size());
        for (int i = 0; i < numToPos.size(); ++i) {
            assert(posToOrigin.at(numToPos.at(i)) == i);
        }
    }

    void print(std::ostream &stream) {
        for (int p : posToOrigin) {
            stream << numbers.at(p) << ' ';
        }
        stream << '\n';
    }
};

} // namespace

template <> void puzzleA<2022, 20>(std::istream &input, std::ostream &output) {
    State state(input);
    state.assertRelation();

    state.mix();
    output << state.sum123k() << '\n';

    // Wrong answer -11930
}

template <> void puzzleB<2022, 20>(std::istream &input, std::ostream &output) {}

} // namespace advent::common
