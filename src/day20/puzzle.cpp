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
    std::vector<int> const numbers;
    std::vector<int> numToPos;
    std::vector<int> posToOrigin;

    State(std::istream &stream)
        : numbers(std::istream_iterator<int>{stream},
                  std::istream_iterator<int>{}),
          numToPos(numbers.size()) {
        std::iota(numToPos.begin(), numToPos.end(), 0);
        posToOrigin = numToPos;
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
        // std::clog << "rot: " << src << ',' << mid << ',' << lst << '\n';
        auto const b = posToOrigin.begin();
        auto const s = b + src;
        auto const m = b + mid;
        auto const l = b + lst;
        std::for_each(s, m, add(l - m));
        std::for_each(m, l, sub(m - s));
        std::rotate(s, m, l);
        assertRelation();
    }
    /*
        void swapFrontBack() {
            // std::clog << "swap\n";
            int &pf = posToOrigin.front();
            int &pb = posToOrigin.back();
            numToPos[pb] = 0;
            numToPos[pf] = numToPos.size() - 1;
            std::swap(pf, pb);
            assertRelation();
        }

        void rotOutOfBounds(int src, int mid, int lst) {
            std::clog << "rot: " << src << ',' << mid << ',' << lst << '\n';
            // Ordered input indices:
            assert(src <= mid);
            assert(mid <= lst);
            if (src == mid || mid == lst)
                return;

            // Limits to "out of bounds":
            const int sz = numToPos.size();
            assert(-sz < src);
            assert(src < sz);
            assert(lst < sz * 2);
            assert(lst - src < sz);

            if (src < 0) {
                src += sz;
                mid += sz;
                lst += sz;
            }
            assert(0 <= src);
            assert(src < sz);

            if (lst <= sz) {
                rotInRange(src, mid, lst);
                return;
            }
            assert(lst > sz);

            if (mid <= sz) {
                assert(src + 1 == mid);
                rotInRange(src, mid, sz);
                swapFrontBack();
                rotInRange(0, 1, lst - sz);
            } else {
                assert(mid + 1 == lst);
                rotInRange(0, mid - sz, lst - sz);
                swapFrontBack();
                rotInRange(src, sz - 1, sz);
            }
        }

        void processOriginalIndex(int i) {
            auto n = numbers.at(i);
            auto pos = numToPos.at(i);
            auto m = mod(pos + n);
            if (i == m)
                return;
            assert(n != 0);

            const int sz = numToPos.size();
            if (n < 0) {
                rotOutOfBounds(m - sz, pos, pos + 1);
            } else if (m < pos) {
                rotOutOfBounds(pos, pos + 1, m + 1 + sz);
            } else {
                rotOutOfBounds(pos, pos + 1, m + 1);
            }
        }
    */
    void processOriginalIndex(int i) {
        auto n = numbers.at(i);
        auto fromPos = numToPos.at(i);
        auto toPos = mod1(fromPos + n);
        // std::clog << "num " << n << ": " << fromPos << " -> " << toPos <<
        // '\n';
        const bool actuallyGoingRight = toPos > fromPos;
        if (actuallyGoingRight) {
            rotInRange(fromPos, fromPos + 1, toPos + 1);
        } else {
            rotInRange(toPos, fromPos, fromPos + 1);
        }
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
    const auto posZeroInInput =
        std::find(state.numbers.begin(), state.numbers.end(), 0);
    if (posZeroInInput == state.numbers.end()) {
        throw std::runtime_error("No zero in input");
    }

    // state.print(std::clog);
    for (int i = 0; i < state.numbers.size(); ++i) {
        state.processOriginalIndex(i);
        // state.print(std::clog);
    }

    const int indexZero = std::distance(state.numbers.begin(), posZeroInInput);
    const int posZero = state.numToPos.at(indexZero);
    int sum = 0;
    for (int offset : {1000, 2000, 3000}) {
        const int p = state.mod(posZero + offset);
        sum += state.numbers.at(state.posToOrigin.at(p));
    }
    const int p1000 = state.mod(posZero + 1000);
    const int p2000 = state.mod(posZero + 2000);
    const int p3000 = state.mod(posZero + 3000);
    // std::clog << state.numbers.at(state.posToOrigin.at(p1000)) << ' '
    //           << state.numbers.at(state.posToOrigin.at(p2000)) << ' '
    //           << state.numbers.at(state.posToOrigin.at(p3000)) << '\n';
    output << sum << '\n';

    // Wrong answer -11930
}

template <> void puzzleB<2022, 20>(std::istream &input, std::ostream &output) {}

} // namespace advent::common
