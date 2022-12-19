#include "adventofcode.hpp"

#include <algorithm>
#include <cassert>
#include <charconv>
#include <iostream>
#include <istream>
#include <iterator>
#include <ostream>
#include <string>
#include <vector>

namespace advent::common {

namespace {

struct ExpectChar {
    char c;
    friend std::istream &operator>>(std::istream &stream, ExpectChar ec) {
        char tmp;
        if (stream >> tmp) {
            if (tmp != ec.c) {
                stream.putback(tmp);
                stream.setstate(std::ios_base::failbit);
            }
        }
        return stream;
    }
};

struct ExpectString {
    std::string str;
    friend std::istream &operator>>(std::istream &stream,
                                    ExpectString const &es) {
        std::string tmp;
        if (stream >> tmp) {
            if (tmp != es.str) {
                stream.setstate(std::ios_base::failbit);
            }
        }
        return stream;
    }
};

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
        return stream >> ExpectChar{'x'} >> ExpectChar{'='} >> pos.col >>
               ExpectChar{','} >> ExpectChar{'y'} >> ExpectChar{'='} >> pos.row;
    }
};

struct Line {
    Index2D sensor, beacon;
    friend std::istream &operator>>(std::istream &stream, Line &line) {
        return stream >> ExpectString{"Sensor"} >> ExpectString{"at"} >>
               line.sensor >> ExpectChar{':'} >> ExpectString{"closest"} >>
               ExpectString{"beacon"} >> ExpectString{"is"} >>
               ExpectString{"at"} >> line.beacon;
    }
};

struct RangedSensor {
    Index2D position;
    std::ptrdiff_t norm;
};

std::ptrdiff_t normManhatten(Index2D p1, Index2D p2) {
    return std::abs(p1.row - p2.row) + std::abs(p1.col - p2.col);
}

RangedSensor makeRanged(Line line) {
    return {line.sensor, normManhatten(line.sensor, line.beacon)};
}

std::pair<std::ptrdiff_t, std::ptrdiff_t> rangeInRow(RangedSensor rs,
                                                     std::ptrdiff_t row) {
    const auto deltaRow = std::abs(rs.position.row - row);
    if (rs.norm < deltaRow) {
        return {0, 0};
    }
    const auto deltaCol = std::abs(rs.norm - deltaRow);
    return {rs.position.col - deltaCol, rs.position.col + deltaCol + 1};
}

} // namespace

template <> void puzzleA<2022, 15>(std::istream &input, std::ostream &output) {
    std::vector sensors(std::istream_iterator<Line>{input},
                        std::istream_iterator<Line>{});

    // static constexpr std::ptrdiff_t targetRow = 10;
    static constexpr std::ptrdiff_t targetRow = 2'000'000;

    std::vector<std::pair<std::ptrdiff_t, std::ptrdiff_t>> ranges;
    std::transform(
        sensors.begin(), sensors.end(), std::back_inserter(ranges),
        [](Line line) { return rangeInRow(makeRanged(line), targetRow); });

    std::ptrdiff_t lowest{}, highest{};
    bool first = true;
    for (auto [low, high] : ranges) {
        if (first) {
            if (low != high) {
                lowest = low;
                highest = high;
                first = false;
            }
        } else {
            lowest = std::min(lowest, low);
            highest = std::max(highest, high);
        }
        //output << lowest << " - " << highest << '\n';
    }

    const std::ptrdiff_t offset = lowest;
    const std::ptrdiff_t length = highest - lowest;

    std::vector<bool> set;
    set.resize(length, false);
    for (auto [low, high] : ranges) {
        std::fill(set.begin() + (low - offset), set.begin() + (high - offset),
                  true);
    }
    for (auto line : sensors) {
        if (line.beacon.row == targetRow) {
            set[line.beacon.col - offset] = false;
        }
    }

    output << std::count(set.begin(), set.end(), true) << '\n';
}

template <> void puzzleB<2022, 15>(std::istream &input, std::ostream &output) {}

} // namespace advent::common
