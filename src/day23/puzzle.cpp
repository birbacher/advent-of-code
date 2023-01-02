#include "adventofcode.hpp"

#include "iosupport/iosupport.hpp"

#include <algorithm>
#include <array>
#include <cassert>
#include <cctype>
#include <charconv>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <istream>
#include <iterator>
#include <numeric>
#include <optional>
#include <ostream>
#include <string>
#include <tuple>
#include <unordered_map>
#include <unordered_set>
#include <variant>
#include <vector>

namespace advent::common {

namespace {

struct Index2D {
    int row{}, col{};

    friend Index2D operator+(Index2D lhs, Index2D rhs) {
        return {lhs.row + rhs.row, lhs.col + rhs.col};
    }
    friend Index2D operator-(Index2D lhs, Index2D rhs) {
        return {lhs.row - rhs.row, lhs.col - rhs.col};
    }
    friend Index2D operator/(Index2D lhs, int rhs) {
        return {lhs.row / rhs, lhs.col / rhs};
    }

    friend bool operator==(Index2D lhs, Index2D rhs) {
        return lhs.row == rhs.row && lhs.col == rhs.col;
    }
    friend bool operator!=(Index2D lhs, Index2D rhs) { return !(lhs == rhs); }

    friend std::ostream &operator<<(std::ostream &stream, Index2D i) {
        return stream << '(' << 'r' << i.row << ' ' << 'c' << i.col << ')';
    }
};

} // namespace
} // namespace advent::common

template <> struct std::hash<advent::common::Index2D> {
    std::size_t operator()(advent::common::Index2D pos) const {
        constexpr long long factor = 1ll << 32;
        std::hash<long long> h{};
        return h(pos.row * factor + pos.col);
    }
};

namespace advent::common {
namespace {

enum Direction {
    dirE,
    dirSE,
    dirS,
    dirSW,
    dirW,
    dirNW,
    dirN,
    dirNE,
};
constexpr Direction allDirs[] = {
    dirE, dirSE, dirS, dirSW, dirW, dirNW, dirN, dirNE,
};

Index2D getOffsetsForDir(Direction dir) {
    switch (dir) {
    case dirE:
        return {0, 1};
    case dirSE:
        return {1, 1};
    case dirS:
        return {1, 0};
    case dirSW:
        return {1, -1};
    case dirW:
        return {0, -1};
    case dirNW:
        return {-1, -1};
    case dirN:
        return {-1, 0};
    case dirNE:
        return {-1, 1};
    }
    throw std::runtime_error("Unknown direction in getOffsetsForDir");
}

using Probe = std::array<bool, 8>;

Probe getProbe(std::unordered_set<Index2D> const &s, Index2D const pos) {
    Probe result{};
    std::transform(std::begin(allDirs), std::end(allDirs), result.begin(),
                   [&s, pos](Direction dir) -> bool {
                       return s.count(pos + getOffsetsForDir(dir));
                   });
    return result;
}

struct State {
    std::vector<Direction> consideredDirections{dirN, dirS, dirW, dirE};

    std::unordered_set<Index2D> elves;

    std::unordered_map<Index2D, Index2D> proposed;

    void read(std::istream &stream) {
        Index2D pos{};
        iosupport::loadFileInLines(stream, [this, &pos](std::string_view sv) {
            pos.col = 0;
            for (char c : sv) {
                if (c == '#') {
                    elves.insert(pos);
                }
                ++pos.col;
            }
            ++pos.row;
        });
        //std::clog << "Elves: " << elves.size() << '\n';
    }

    static bool testPossibleProposal(Probe const &p, Direction d) {
        const auto i = static_cast<int>(d);
        const auto j = (i + 1) % 8;
        const auto k = (i + 7) % 8;
        return p.at(i) == 0 && p.at(j) == 0 && p.at(k) == 0;
    }

    std::optional<Index2D> elfFindProposed(Index2D pos) const {
        Probe const p = getProbe(elves, pos);
        if (std::count(p.begin(), p.end(), 0) == p.size()) {
            return std::nullopt;
        }
        for (Direction d : consideredDirections) {
            if (testPossibleProposal(p, d)) {
                return pos + getOffsetsForDir(d);
            }
        }
        return std::nullopt;
    }

    void findAllProposed() {
        proposed.clear();
        std::unordered_set<Index2D> collisions;
        for (Index2D e : elves) {
            if (auto x = elfFindProposed(e)) {
                if (collisions.count(*x)) {
                    continue;
                }
                if (proposed.count(*x)) {
                    collisions.insert(*x);
                    proposed.erase(*x);
                } else {
                    proposed.emplace(*x, e);
                }
            }
        }
    }

    void moveProposed() {
        for (auto [to, from] : proposed) {
            elves.erase(from);
            elves.insert(to);
        }
    }

    void rotateConsideredDirections() {
        std::rotate(consideredDirections.begin(),
                    consideredDirections.begin() + 1,
                    consideredDirections.end());
    }

    void runRound() {
        findAllProposed();
        moveProposed();
        rotateConsideredDirections();
    }

    std::pair<Index2D, Index2D> getBounding() const {
        Index2D min = *elves.begin(), max = min;
        for (auto [r, c] : elves) {
            min.row = std::min(min.row, r);
            min.col = std::min(min.col, c);
            max.row = std::max(max.row, r);
            max.col = std::max(max.col, c);
        }
        return {min, max};
    }

    int countSpace() const {
        auto [min, max] = getBounding();
        return (max.row - min.row + 1) * (max.col - min.col + 1) - elves.size();
    }

    void print(std::ostream &stream) const {
        auto [min, max] = getBounding();
        for (int row = min.row; row <= max.row; ++row) {
            for (int col = min.col; col <= max.col; ++col) {
                stream << (elves.count(Index2D{row, col}) ? '#' : '.');
            }
            stream << '\n';
        }
    }
};

} // namespace

template <> void puzzleA<2022, 23>(std::istream &input, std::ostream &output) {
    State s;
    s.read(input);
    // s.print(std::clog);
    for (int i = 0; i < 10; ++i) {
        s.runRound();
        // s.print(std::clog << "\n\n");
    }
    output << s.countSpace() << '\n';
}

template <> void puzzleB<2022, 23>(std::istream &input, std::ostream &output) {}

} // namespace advent::common
