#include "adventofcode.hpp"

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
#include <ostream>
#include <string>
#include <tuple>
#include <variant>
#include <vector>

namespace advent::common {

namespace {

struct Index2D {
    int row{}, col{};

    friend Index2D operator+(Index2D lhs, Index2D rhs) {
        return {lhs.row + rhs.row, lhs.col + rhs.col};
    }

    friend std::ostream &operator<<(std::ostream &stream, Index2D i) {
        return stream << '(' << 'r' << i.row << ' ' << 'c' << i.col << ')';
    }
};

struct Field {
    std::vector<std::string> rows;

    char operator[](Index2D i) const { return rows.at(i.row).at(i.col); }

    friend std::istream &operator>>(std::istream &stream, Field &field) {
        field.rows.clear();
        std::string line;
        while (std::getline(stream, line)) {
            if (line.empty())
                break;
            field.rows.push_back(std::move(line));
        }
        return stream;
    }
};

enum Turn {
    turnR,
    turnL,
};

using Move = std::variant<int, Turn>;

struct Moves {
    std::vector<Move> moves;

    friend std::istream &operator>>(std::istream &stream, Moves &moves) {
        moves.moves.clear();
        char tmp;
        while (stream >> tmp) {
            if (std::isdigit(tmp)) {
                stream.putback(tmp);
                stream >> moves.moves.emplace_back().emplace<int>();
            } else if (tmp == 'R') {
                moves.moves.emplace_back().emplace<Turn>(turnR);
            } else if (tmp == 'L') {
                moves.moves.emplace_back().emplace<Turn>(turnL);
            } else {
                stream.putback(tmp);

                break;
            }
        }
        return stream;
    }
};

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

struct State {
    Field f;
    Index2D pos;
    Direction dir{};

    void findStart() {
        std::size_t len = 0;
        for (auto const &r : f.rows) {
            len = std::max(len, r.size());
        }
        for (auto &r : f.rows) {
            r.resize(len, ' ');
        }
        pos.row = 0;
        pos.col = f.rows.at(0).find_first_not_of(" #");
        dir = dirR;
    }

    void turn(Turn t) {
        int d = static_cast<int>(dir);
        int delta = t == turnL ? -1 : 1;
        d = (d + 4 + delta) % 4;
        dir = static_cast<Direction>(d);
    }

    Index2D wrap(Index2D i) {
        return {mod(i.row, f.rows.size()), mod(i.col, f.rows.at(0).size())};
    }

    bool step() {
        const auto offset = getOffsetsForDir(dir);
        Index2D nextPos = pos;
    again:
        nextPos = wrap(nextPos + offset);
        switch (f[nextPos]) {
        case ' ':
            goto again;
        case '.':
            pos = nextPos;
            return true;
        case '#':
            return false;
        }
        throw std::runtime_error("Unknown character in input");
    }

    void move(Move m) {
        if (int *p = std::get_if<int>(&m)) {
            for (int i = 0; i < *p; ++i) {
                if (!step())
                    break;
            }
            //std::clog << pos << ' ' << int(dir) << '\n';
        } else if (Turn *t = std::get_if<Turn>(&m)) {
            turn(*t);
        }
    }

    int answer() const {
        return pos.row * 1000 + pos.col * 4 + int(dir) + 1004;
    }
};

} // namespace

template <> void puzzleA<2022, 22>(std::istream &input, std::ostream &output) {
    State state;
    Moves moves;
    input >> state.f >> moves;
    state.findStart();
    //std::clog << "starts at: " << state.pos << '\n';
    for (auto m : moves.moves) {
        state.move(m);
    }
    output << state.answer() << '\n';
}

template <> void puzzleB<2022, 22>(std::istream &input, std::ostream &output) {}

} // namespace advent::common
