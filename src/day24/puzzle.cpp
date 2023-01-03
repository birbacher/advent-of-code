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

struct Dim {
    int columns{};
    int rows{};
};

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
    dirR, // right
    dirD, // down
    dirL, // left
    dirU, // up
};

Index2D getOffsetsForDir(Direction dir) {
    switch (dir) {
    case dirR:
        return {0, 1};
    case dirL:
        return {0, -1};
    case dirD:
        return {1, 0};
    case dirU:
        return {-1, 0};
    }
    throw std::runtime_error("Unknown direction in getOffsetsForDir");
}

int mod(int a, int b) {
    auto res = a % b;
    return res >= 0 ? res : res + b;
}

Index2D mod(Index2D pos, Dim dim) {
    return {mod(pos.row, dim.rows), mod(pos.col, dim.columns)};
}

template <typename T> struct FieldT {
    std::vector<T> data;
    Dim dim;

    FieldT(Dim d, T t = T{}) : data(d.rows * d.columns, t), dim(d) {}

    T &operator[](Index2D pos) {
        return data.at(pos.row * dim.columns + pos.col);
    }
    T const &operator[](Index2D pos) const {
        return data.at(pos.row * dim.columns + pos.col);
    }
};

struct Blizzard {
    Index2D pos;
    Direction dir = dirR;
};

enum FieldState { eReachable, eBlocked };

using Field = FieldT<FieldState>;

struct Plain {
    Dim dim;
    std::vector<Blizzard> blizzards;

    Index2D start;
    Index2D goal;

    void moveBlizzards() {
        for (Blizzard &b : blizzards) {
            b.pos = mod(b.pos + getOffsetsForDir(b.dir), dim);
        }
    }
};

Direction dirFromChar(char c) {
    switch (c) {
    case '>':
        return dirR;
    case 'v':
        return dirD;
    case '<':
        return dirL;
    case '^':
        return dirU;
    }
    throw std::runtime_error("Unknown char in dirFromChar");
}

char charFromDir(Direction d) {
    switch (d) {
    case dirR:
        return '>';
    case dirD:
        return 'v';
    case dirL:
        return '<';
    case dirU:
        return '^';
    }
    throw std::runtime_error("Unknown direction in charFromDir");
}

Plain readPlain(std::istream &stream) {
    Plain plain;
    std::string tmp;
    if (std::getline(stream, tmp)) {
        if (tmp.size() < 4 ||
            std::count(tmp.begin(), tmp.end(), '#') != tmp.size() - 1 ||
            tmp.at(1) != '.') {
            goto exitfailure;
        }
        plain.dim.columns = tmp.size() - 2;

        while (std::getline(stream, tmp)) {
            if (tmp.size() != plain.dim.columns + 2 || tmp.front() != '#' ||
                tmp.back() != '#') {
                goto exitfailure;
            }
            if (tmp.at(1) == '#') {
                if (tmp.rbegin()[1] != '.' ||
                    std::count(tmp.begin(), tmp.end(), '#') != tmp.size() - 1) {
                    goto exitfailure;
                }
                break;
            }
            for (int i = 0; i < plain.dim.columns; ++i) {
                const char c = tmp.at(i + 1);
                if (c != '.') {
                    plain.blizzards.emplace_back(
                        Blizzard{{plain.dim.rows, i}, dirFromChar(c)});
                }
            }
            ++plain.dim.rows;
        }
    }
    return plain;
exitfailure:
    stream.setstate(std::ios_base::failbit);
    return plain;
}

void fillReachableByPrevious(Field &f, Index2D prev) {
    f[prev] = eReachable;
    if (prev.col > 0) {
        f[prev + Index2D{0, -1}] = eReachable;
    }
    if (prev.col + 1 < f.dim.columns) {
        f[prev + Index2D{0, 1}] = eReachable;
    }
    if (prev.row > 0) {
        f[prev + Index2D{-1, 0}] = eReachable;
    }
    if (prev.row + 1 < f.dim.rows) {
        f[prev + Index2D{1, 0}] = eReachable;
    }
}

void fillReachableByPrevious(Field &f, Field const &prev) {
    Index2D pos;
    for (pos.row = 0; pos.row < f.dim.rows; ++pos.row) {
        for (pos.col = 0; pos.col < f.dim.columns; ++pos.col) {
            if (prev[pos] == eReachable) {
                fillReachableByPrevious(f, pos);
            }
        }
    }
    for (auto const &p : prev.data) {
    }
}

void fillBlockedByBlizzards(Field &f, Plain const &p) {
    for (auto const &b : p.blizzards) {
        f[b.pos] = eBlocked;
    }
}

void updateField(Field &f, Plain const &p) {
    // Start with a completely blocked field:
    Field next(f.dim, eBlocked);

    // Mark as reachable the places we can go from 'f':
    next[p.start] = eReachable; // reachable from start position
    fillReachableByPrevious(next, f);

    // Overwrite everything that is blocked by a blizzard:
    fillBlockedByBlizzards(next, p);

    f = std::move(next);
}

bool isExitReachable(Field const &f) {
    return f[Index2D{f.dim.rows - 1, f.dim.columns - 1}] == eReachable;
}
bool isExitReachable(Field const &f, Index2D pos) {
    return f[pos] == eReachable;
}

} // namespace

template <> void puzzleA<2022, 24>(std::istream &input, std::ostream &output) {
    Plain p = readPlain(input);
    if (input.fail()) {
        throw std::runtime_error("Error reading input");
    }
    std::clog << "Plain " << p.dim.rows << 'x' << p.dim.columns << " with "
              << p.blizzards.size() << " blizzards\n";

    p.start = Index2D{0, 0};
    p.goal = Index2D{p.dim.rows - 1, p.dim.columns - 1};
    Field f(p.dim, eBlocked);
    int steps;
    for (steps = 1; !isExitReachable(f, p.goal); ++steps) {
        p.moveBlizzards();
        updateField(f, p);
    }
    output << steps << '\n';
}

template <> void puzzleB<2022, 24>(std::istream &input, std::ostream &output) {
    Plain p = readPlain(input);
    if (input.fail()) {
        throw std::runtime_error("Error reading input");
    }
    std::clog << "Plain " << p.dim.rows << 'x' << p.dim.columns << " with "
              << p.blizzards.size() << " blizzards\n";

    p.start = Index2D{0, 0};
    p.goal = Index2D{p.dim.rows - 1, p.dim.columns - 1};
    int steps = 1;
    for (int i = 0; i < 3; ++i) {
        Field f(p.dim, eBlocked);
        for (; !isExitReachable(f, p.goal); ++steps) {
            p.moveBlizzards();
            updateField(f, p);
        }
        std::swap(p.start, p.goal);
    }
    output << steps << '\n';
}

} // namespace advent::common
