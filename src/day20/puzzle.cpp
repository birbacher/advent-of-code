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

constexpr std::int64_t decryptionKey = 811589153;

std::vector<std::int64_t> readInput(std::istream &stream, std::int64_t key) {
    std::vector numbers(std::istream_iterator<std::int64_t>{stream},
                        std::istream_iterator<std::int64_t>{});
    std::transform(numbers.begin(), numbers.end(), numbers.begin(),
                   [key](std::int64_t v) { return key * v; });
    return numbers;
}

struct State {
    const std::vector<std::int64_t> numbers;
    const int indexZero{};
    std::vector<int> numToPos;
    std::vector<int> posToOrigin;

    State(std::istream &stream, std::int64_t key = 1)
        : numbers(readInput(stream, key)),
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
    int mod1(std::int64_t value) {
        std::int64_t N = static_cast<std::int64_t>(numToPos.size()) - 1;
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
        std::int64_t n = numbers.at(i);
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

    std::int64_t sum123k() {
        const int posZero = numToPos.at(indexZero);
        std::int64_t sum = 0;
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

template <> void puzzleB<2022, 20>(std::istream &input, std::ostream &output) {
    State state(input, decryptionKey);
    state.assertRelation();

    for (int i = 0; i < 10; ++i) {
        state.mix();
    }
    output << state.sum123k() << '\n';
}

} // namespace advent::common
