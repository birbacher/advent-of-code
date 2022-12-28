#include "adventofcode.hpp"

#include "iosupport/iosupport.hpp"

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

using iosupport::literals::operator""_e;

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
        return stream >> 'x'_e >> '='_e >> pos.col >> ','_e >> 'y'_e >> '='_e >>
               pos.row;
    }
};

struct Line {
    Index2D sensor, beacon;
    friend std::istream &operator>>(std::istream &stream, Line &line) {
        return stream >> "Sensor"_e >> "at"_e >> line.sensor >> ':'_e >>
               "closest"_e >> "beacon"_e >> "is"_e >> "at"_e >> line.beacon;
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

std::vector<Line> loadSensors(std::istream &input) {
    return std::vector(std::istream_iterator<Line>{input},
                       std::istream_iterator<Line>{});
}

std::vector<std::pair<std::ptrdiff_t, std::ptrdiff_t>>
loadRanges(std::vector<Line> const &sensors, std::ptrdiff_t targetRow) {
    std::vector<std::pair<std::ptrdiff_t, std::ptrdiff_t>> ranges;
    std::transform(sensors.begin(), sensors.end(), std::back_inserter(ranges),
                   [targetRow](Line line) {
                       return rangeInRow(makeRanged(line), targetRow);
                   });
    return ranges;
}

std::pair<std::ptrdiff_t, std::ptrdiff_t> findLowHigh(
    std::vector<std::pair<std::ptrdiff_t, std::ptrdiff_t>> const &ranges) {
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
    }
    return {lowest, highest};
}

std::vector<bool> loadPositions(
    std::vector<Line> const &sensors,
    std::vector<std::pair<std::ptrdiff_t, std::ptrdiff_t>> const &ranges,
    std::ptrdiff_t lowest, std::ptrdiff_t highest, std::ptrdiff_t targetRow) {
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
    return set;
}

} // namespace

template <> void puzzleA<2022, 15>(std::istream &input, std::ostream &output) {
    // static constexpr std::ptrdiff_t targetRow = 10;
    static constexpr std::ptrdiff_t targetRow = 2'000'000;

    const std::vector sensors = loadSensors(input);
    const std::vector ranges = loadRanges(sensors, targetRow);

    auto const [lowest, highest] = findLowHigh(ranges);

    const std::vector<bool> set =
        loadPositions(sensors, ranges, lowest, highest, targetRow);

    output << std::count(set.begin(), set.end(), true) << '\n';
}

template <> void puzzleB<2022, 15>(std::istream &input, std::ostream &output) {
    // static constexpr std::ptrdiff_t targetRow = 10;
    // static constexpr std::ptrdiff_t minCo = 0;
    // static constexpr std::ptrdiff_t maxCo = 20;
    static constexpr std::ptrdiff_t targetRow = 2'000'000;
    static constexpr std::ptrdiff_t minCo = 0;
    static constexpr std::ptrdiff_t maxCo = 4'000'000;
    static constexpr std::ptrdiff_t resultFactor = 4'000'000;

    const std::vector sensors = loadSensors(input);
    for (std::ptrdiff_t y = minCo; y <= maxCo; ++y) {
        std::vector ranges = loadRanges(sensors, y);
        std::sort(ranges.begin(), ranges.end());
        std::ptrdiff_t curX = minCo;
        for (auto [l, h] : ranges) {
            if (curX < l) {
                output << (curX * resultFactor + y) << '\n';
                return;
            }
            curX = std::max(curX, h);
            if (curX > maxCo) {
                break;
            }
        }
    }
    throw std::runtime_error("Exhausted search space");

    // RelWithDebInfo
    // runtime: real    0m5,843s
}

} // namespace advent::common
