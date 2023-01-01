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
    friend Index2D operator-(Index2D lhs, Index2D rhs) {
        return {lhs.row - rhs.row, lhs.col - rhs.col};
    }
    friend Index2D operator/(Index2D lhs, int rhs) {
        return {lhs.row / rhs, lhs.col / rhs};
    }

    friend std::ostream &operator<<(std::ostream &stream, Index2D i) {
        return stream << '(' << 'r' << i.row << ' ' << 'c' << i.col << ')';
    }
};

struct Field {
    std::vector<std::string> rows;

    char operator[](Index2D i) const { return rows.at(i.row).at(i.col); }
    char &operator[](Index2D i) { return rows.at(i.row).at(i.col); }

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

Direction turnDir(Turn t, Direction dir) {
    int d = static_cast<int>(dir);
    int delta = t == turnL ? -1 : 1;
    d = (d + 4 + delta) % 4;
    return static_cast<Direction>(d);
}

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
    int tileSize{};

    void findStart() {
        std::size_t len = 0;
        for (auto const &r : f.rows) {
            len = std::max(len, r.size());
        }
        for (auto &r : f.rows) {
            r.resize(len, ' ');
        }
        tileSize = std::gcd(f.rows.size(), f.rows.at(0).size());
        pos.row = 0;
        pos.col = f.rows.at(0).find_first_not_of(" #");
        dir = dirR;
    }

    void fillTiles() {
        std::clog << "gcd: " << tileSize << '\n';
        Index2D t{};
        char const *p = "ABCDEFGHIJ";
        f.rows.resize(f.rows.size() + tileSize,
                      std::string(f.rows.at(0).size(), ' '));
        for (; t.row < f.rows.size(); t.row += tileSize) {
            for (t.col = 0; t.col < f.rows.at(0).size(); t.col += tileSize) {
                if (f[t] != ' ')
                    continue;
                Index2D w{};
                for (w.row = 0; w.row < tileSize; ++w.row) {
                    for (w.col = 0; w.col < tileSize; ++w.col) {
                        f[t + w] = *p;
                    }
                }
                ++p;
            }
        }
    }

    std::pair<Index2D, Index2D> split(Index2D i) {
        const Index2D offset{i.row % tileSize, i.col % tileSize};
        return {i - offset, offset};
    }

    Index2D rotL(Index2D i) const { return {tileSize - i.col - 1, i.row}; }
    Index2D rotR(Index2D i) const { return {i.col, tileSize - i.row - 1}; }
    Index2D rotU(Index2D i) const { return rotL(rotL(i)); }
    Index2D onSq(int r, int c) const { return {r * tileSize, c * tileSize}; }

    std::pair<Index2D, Direction> cube(Index2D i, Direction d) {
        {
            const auto [tile, offset] = split(i);
            std::clog << "trace pos " << i << " off " << offset << " dir " << d
                      << '\n';
        }
        std::tie(i, d) = cube_(i, d);
        {
            const auto [tile, offset] = split(i);
            std::clog << "   to pos " << i << " off " << offset << " dir " << d
                      << '\n';
        }
        return {i, d};
    }
    std::pair<Index2D, Direction> cube_(Index2D i, Direction d) {
        const auto [tile, offset] = split(i);
        // hardcode layout:
        if (tileSize == 4) {
            // test data
            switch (f[i]) {
            case 'A': {
                if (d == dirU)
                    return {rotU(offset) + onSq(0, 2), dirD};
            }
            case 'B': {
                switch (d) {
                case dirU:
                    return {rotR(offset) + onSq(0, 2), dirR};
                case dirL:
                    return {rotL(offset) + onSq(1, 1), dirU};
                }
                break;
            }
            case 'C': {
                switch (d) {
                case dirR:
                    return {rotU(offset) + onSq(2, 3), dirL};
                }
                break;
            }
            case 'D': {
                switch (d) {
                case dirR:
                    return {rotR(offset) + onSq(2, 3), dirD};
                case dirU:
                    return {rotL(offset) + onSq(1, 2), dirL};
                case dirL:
                    return {rotR(offset) + onSq(2, 3), dirU};
                }
                break;
            }
            case 'E': {
                switch (d) {
                case dirR:
                    return {rotU(offset) + onSq(0, 2), dirL};
                case dirD:
                    return {rotU(offset) + onSq(2, 2), dirU};
                }
                break;
            }
            case 'F': {
                switch (d) {
                case dirL:
                    return {rotR(offset) + onSq(1, 1), dirU};
                case dirD:
                    return {rotL(offset) + onSq(2, 2), dirR};
                }
                break;
            }
            case 'I': {
                switch (d) {
                case dirD:
                    return {rotU(offset) + onSq(1, 0), dirU};
                case dirU:
                    return {rotU(offset) + onSq(1, 0), dirD};
                }
                break;
            }
            case 'J': {
                switch (d) {
                case dirD:
                    return {rotU(offset) + onSq(1, 0), dirU};
                }
                break;
            }
            }
        } else if (tileSize == 50) {
            // real data
            switch (f[i]) {
            case 'A': {
                switch (d) {
                case dirR:
                    return {rotU(offset) + onSq(2, 1), dirL};
                case dirL:
                    return {rotU(offset) + onSq(2, 0), dirR};
                }
                break;
            }
            case 'B': {
                switch (d) {
                case dirL:
                    return {rotL(offset) + onSq(2, 0), dirD};
                case dirU:
                    return {rotR(offset) + onSq(2, 0), dirR};
                }
                break;
            }
            case 'C': {
                switch (d) {
                case dirR:
                    return {rotL(offset) + onSq(0, 2), dirU};
                case dirD:
                    return {rotR(offset) + onSq(1, 1), dirL};
                }
                break;
            }
            case 'D': {
                switch (d) {
                case dirR:
                    return {rotU(offset) + onSq(0, 2), dirL};
                case dirL:
                    return {rotU(offset) + onSq(0, 1), dirR};
                }
                break;
            }
            case 'E': {
                switch (d) {
                case dirR:
                    return {rotL(offset) + onSq(2, 1), dirU};
                case dirD:
                    return {rotR(offset) + onSq(3, 0), dirL};
                }
                break;
            }
            case 'F': {
                switch (d) {
                case dirL:
                    return {rotL(offset) + onSq(0, 1), dirD};
                }
                break;
            }
            case 'G': {
                switch (d) {
                case dirD:
                    return {offset + onSq(0, 2), dirD};
                }
                break;
            }
            case 'H': {
                switch (d) {
                case dirU:
                    return {rotR(offset) + onSq(3, 0), dirR};
                }
                break;
            }
            case 'I': {
                switch (d) {
                case dirU:
                    return {offset + onSq(3, 0), dirU};
                }
                break;
            }
            }
        } else {
            throw std::runtime_error("Unsupported tile size");
        }
        std::clog << "pos " << i << '[' << f[i] << "] off " << offset
                  << " tile " << (tile / tileSize) << " dir " << d << '\n';
        throw std::runtime_error("invalid wrap");
    }

    void turn(Turn t) { dir = turnDir(t, dir); }

    Index2D wrap(Index2D i) {
        return {mod(i.row, f.rows.size()), mod(i.col, f.rows.at(0).size())};
    }

    bool step() {
        auto offset = getOffsetsForDir(dir);
        Index2D nextPos = pos;
        Direction nextDir = dir;
    doStep:
        nextPos = wrap(nextPos + offset);
    doCheck:
        std::clog << "trace step " << nextPos << " in " << nextDir << '\n';
        switch (f[nextPos]) {
        case ' ':
            goto doStep;
        case '.':
            pos = nextPos;
            dir = nextDir;
            return true;
        case '#':
            return false;
        case 'A':
        case 'B':
        case 'C':
        case 'D':
        case 'E':
        case 'F':
        case 'G':
        case 'H':
        case 'I':
        case 'J':
            std::tie(nextPos, nextDir) = cube(nextPos, dir);
            offset = getOffsetsForDir(nextDir);
            goto doCheck;
        }
        throw std::runtime_error("Unknown character in input");
    }

    void move(Move m) {
        if (int *p = std::get_if<int>(&m)) {
            for (int i = 0; i < *p; ++i) {
                std::clog << "  Move " << *p << '\n';
                if (!step())
                    break;
            }
            // std::clog << pos << ' ' << int(dir) << '\n';
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
    // std::clog << "starts at: " << state.pos << '\n';
    for (auto m : moves.moves) {
        state.move(m);
    }
    output << state.answer() << '\n';
}

template <> void puzzleB<2022, 22>(std::istream &input, std::ostream &output) {
    State state;
    Moves moves;
    input >> state.f >> moves;
    state.findStart();
    state.fillTiles();
    for (auto const &r : state.f.rows) {
        std::clog << r << '\n';
    }
    // std::clog << "starts at: " << state.pos << '\n';
    for (auto m : moves.moves) {
        state.move(m);
    }
    output << state.answer() << '\n';

    // Answer is too low: 7396
}

} // namespace advent::common
