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

enum Direction {
    Left,
    Right,
    Up,
    Down,
};

Direction recognizeDirection(char c) {
    switch (c) {
    case 'L':
        return Left;
    case 'R':
        return Right;
    case 'U':
        return Up;
    case 'D':
        return Down;
    }
    using namespace std::literals;
    throw std::runtime_error("Unknown direction character: "s + c);
}

std::istream &operator>>(std::istream &stream, Direction &direction) {
    char c;
    if (stream >> c) {
        direction = recognizeDirection(c);
    }
    return stream;
}

struct Move {
    Direction dir = Left;
    int amount{};
};

std::istream &operator>>(std::istream &stream, Move &m) {
    return stream >> m.dir >> m.amount;
}

struct Position {
    /// @brief Horizontal position, positive values go right.
    int h{};
    /// @brief Vertical position, positive values go up.
    int v{};

    friend bool operator==(Position const &lhs, Position const &rhs) {
        return lhs.h == rhs.h && lhs.v == rhs.v;
    }
};

struct PosHash {
    std::size_t operator()(Position p) const {
        return std::hash<long long>()((long long)p.h * p.v);
    }
};

Position dragTail(Position head, Position tail) {
    int absH = std::abs(head.h - tail.h);
    int absV = std::abs(head.v - tail.v);
    if (absH <= 1 && absV <= 1) {
        return tail;
    }
    int dragH = head.h > tail.h ? head.h - 1 : head.h + 1;
    int dragV = head.v > tail.v ? head.v - 1 : head.v + 1;
    if (head.h == tail.h) {
        return {head.h, dragV};
    }
    if (head.v == tail.v) {
        return {dragH, head.v};
    }
    if (absH == 2 && absV == 2) {
        return {dragH, dragV};
    } else if (absH == 2) {
        return {dragH, head.v};
    } else {
        return {head.h, dragV};
    }
}

Position updatePosition(Position p, Direction dir) {
    switch (dir) {
    case Left:
        return {p.h - 1, p.v};
    case Right:
        return {p.h + 1, p.v};
    case Up:
        return {p.h, p.v + 1};
    case Down:
        return {p.h, p.v - 1};
    }
    throw std::domain_error("Invalid enum value");
}

bool consumeMoveOne(Position &pos, Move &m) {
    if (m.amount < 0) {
        throw std::domain_error(
            "Move amount must be positive for consumeMoveOne()");
    }
    if (m.amount == 0) {
        return false;
    }
    pos = updatePosition(pos, m.dir);
    --m.amount;
    return true;
}

struct Tracker {
    std::unordered_set<Position, PosHash> visited;
    std::vector<Position> rope;

    Tracker(std::size_t n = 2) : rope(n) {
        assert(n >= 2);
        visited.insert(rope.back()); // starting position
    }

    void applyMove(Move m) {
        while (consumeMoveOne(rope.front(), m)) {
            for (std::size_t i = 1; i < rope.size(); ++i) {
                rope[i] = dragTail(rope.at(i - 1), rope.at(i));
            }
            visited.insert(rope.back());
        }
    }

    std::size_t numVisited() const { return visited.size(); }
};

} // namespace

template <> void puzzleA<2022, 9>(std::istream &input, std::ostream &output) {
    Tracker t;
    std::for_each(std::istream_iterator<Move>(input),
                  std::istream_iterator<Move>(),
                  [&](Move m) { t.applyMove(m); });
    output << t.numVisited() << '\n';
}

template <> void puzzleB<2022, 9>(std::istream &input, std::ostream &output) {
    Tracker t(10);
    std::for_each(std::istream_iterator<Move>(input),
                  std::istream_iterator<Move>(),
                  [&](Move m) { t.applyMove(m); });
    output << t.numVisited() << '\n';
}

// 2466
// That's not the right answer; your answer is too low.

} // namespace advent::common
