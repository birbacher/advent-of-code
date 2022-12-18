#include "adventofcode.hpp"

#include <algorithm>
#include <cassert>
#include <charconv>
#include <istream>
#include <iterator>
#include <ostream>
#include <string>
#include <vector>
#include <iostream>

namespace advent::common {

namespace {

struct Index2D {
    std::ptrdiff_t row{}, col{};

    friend bool operator==(Index2D lhs, Index2D rhs) {
        return lhs.row == rhs.row && lhs.col == rhs.col;
    }
    friend bool operator!=(Index2D lhs, Index2D rhs) { return !(lhs == rhs); }
    friend std::ostream &operator<<(std::ostream &stream, Index2D pos) {
        return stream << '(' << 'r' << pos.row << ' ' << 'c' << pos.col << ')';
    }
    friend std::istream &operator>>(std::istream &stream, Index2D &pos) {
        char tmp;
        if (stream >> pos.col >> tmp) {
            if (tmp != ',') {
                stream.putback(tmp);
                stream.setstate(std::ios_base::failbit);
            } else {
                stream >> pos.row;
            }
        }
        return stream;
    }
};

struct Leg {
    bool draw{};
    Index2D toPos;

    friend std::ostream &operator<<(std::ostream &stream, Leg leg) {
        return stream << (leg.draw ? "draw" : "move") << " to " << leg.toPos;
    }

    friend std::istream &operator>>(std::istream &stream, Leg &leg) {
        char tmp;

        //////////////////////
        // Read first character of "->", possibly
        if (!(stream >> tmp)) {
            return stream;
        }

        if (std::isdigit(tmp)) {
            leg.draw = false;
            stream.putback(tmp);
            goto readPos;
        }
        if (tmp != '-') {
            stream.putback(tmp);
            stream.setstate(std::ios_base::failbit);
            return stream;
        }

        //////////////////////
        // Read second character of "->"
        if (!(stream >> tmp)) {
            return stream;
        }

        if (tmp != '>') {
            stream.putback(tmp);
            stream.setstate(std::ios_base::failbit);
            return stream;
        }

        leg.draw = true;

        //////////////////////
        // Read position
    readPos:
        return stream >> leg.toPos;
    }
};

struct Extends {
    Index2D lowest, highest;

    explicit Extends(Index2D initial) : lowest(initial), highest(initial) {}

    void record(Index2D pos) {
        lowest.row = std::min(lowest.row, pos.row);
        lowest.col = std::min(lowest.col, pos.col);
        highest.row = std::max(highest.row, pos.row);
        highest.col = std::max(highest.col, pos.col);
    }
    void increase() {
        //--lowest.row;
        --lowest.col;
        ++highest.row;
        ++highest.col;
    }
};

constexpr Index2D sandSource{0, 500};

struct Dim {
    std::ptrdiff_t columns{};
    std::ptrdiff_t rows{};
};

template <typename T> struct FieldT {
    std::vector<T> data;
    Dim dim;

    FieldT(Dim dim) : data(dim.columns * dim.rows), dim(dim) {}

    T &operator[](Index2D pos) {
        return data.at(pos.row * dim.columns + pos.col);
    }
};

enum SandState {
    e_Empty,
    e_Wall,
    e_Sand,
};

std::ostream& operator<<(std::ostream& stream, SandState state) {
    switch (state) {
        case e_Empty: return stream << '.';
        case e_Wall: return stream << '#';
        case e_Sand: return stream << 'o';
    }
    throw std::runtime_error("Invalid enum value");
}

void drawWalls(FieldT<SandState> &field, std::vector<Leg> const &legs) {
    std::fill(field.data.begin(), field.data.end(), e_Empty);
    Index2D cur = legs.front().toPos;
    for (Leg l : legs) {
        if (l.draw) {
            if (cur.col == l.toPos.col) {
                std::ptrdiff_t delta = cur.row < l.toPos.row ? 1 : -1;
                for (; cur.row != l.toPos.row; cur.row += delta) {
                    field[cur] = e_Wall;
                }
            } else if (cur.row == l.toPos.row) {
                std::ptrdiff_t delta = cur.col < l.toPos.col ? 1 : -1;
                for (; cur.col != l.toPos.col; cur.col += delta) {
                    field[cur] = e_Wall;
                }
            } else {
                throw std::runtime_error("Movement not along x or y axis");
            }
        }
        field[cur] = e_Wall;
        cur = l.toPos;
    }
}

bool dropSandUnit(FieldT<SandState> &field) {
    Index2D cur = sandSource;
    for (;;) {
        const Index2D prev = cur;
        ++cur.row;
        if (cur.row >= field.dim.rows) {
            return false;
        }
        if (field[cur] == e_Empty) {
            continue;
        }
        --cur.col;
        if (field[cur] == e_Empty) {
            continue;
        }
        cur.col += 2;
        if (field[cur] == e_Empty) {
            continue;
        }
        field[prev] = e_Sand;
        return true;
    }
}

void printField(FieldT<SandState> const& field, std::ostream& stream, std::ptrdiff_t skipCols) {
    auto it = field.data.begin();
    for (; it < field.data.end(); ) {
        auto start = it + skipCols;
        it += field.dim.columns;
        std::copy(start, it, std::ostream_iterator<SandState>(stream));
        stream << '\n';
    }
}

} // namespace

template <> void puzzleA<2022, 14>(std::istream &input, std::ostream &output) {
    std::vector<Leg> legs(std::istream_iterator<Leg>{input},
                          std::istream_iterator<Leg>{});
    Extends e{sandSource};
    std::for_each(legs.begin(), legs.end(),
                  [&e](Leg leg) { e.record(leg.toPos); });
    e.increase();

    // We won't optimise skipping the empty cols zero to lowest.col:
    if (e.lowest.row != sandSource.row) {
        throw std::runtime_error(
            "Encountered a row number above the source of sand");
    }
    FieldT<SandState> field(Dim{e.highest.col, e.highest.row});
    drawWalls(field, legs);
    int num = 0;
    while (dropSandUnit(field))
        ++num;

    //printField(field, std::clog, e.lowest.col);

    output << num << '\n';
}

template <> void puzzleB<2022, 14>(std::istream &input, std::ostream &output) {}

} // namespace advent::common
