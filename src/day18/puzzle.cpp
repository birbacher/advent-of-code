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

ExpectChar operator""_e(char c) { return {c}; }

struct Index3D {
    int x{}, y{}, z{};

    void rot() {
        int t = x;
        x = y;
        y = z;
        z = t;
    }

    std::tuple<int, int, int> p() const { return {x, y, z}; }

    friend bool operator<(Index3D lhs, Index3D rhs) {
        return lhs.p() < rhs.p();
    }

    friend std::ostream &operator<<(std::ostream &stream, Index3D i) {
        return stream << '(' << i.x << ',' << i.y << ',' << i.z << ')';
    }
    friend std::istream &operator>>(std::istream &stream, Index3D &i) {
        return stream >> i.x >> ','_e >> i.y >> ','_e >> i.z;
    }
};

Index3D maxCoord(std::vector<Index3D> const &vec) {
    Index3D result;
    for (Index3D const &i : vec) {
        if (i.x < 0 || i.y < 0 || i.z < 0) {
            throw std::runtime_error(
                "Negative numbers in input aren't supported");
        }
        result.x = std::max(result.x, i.x);
        result.y = std::max(result.y, i.y);
        result.z = std::max(result.z, i.z);
    }
    return result;
}

using Row = std::uint32_t;

struct OneSide {
    OneSide(Index3D i) : columns(i.x + 1) {}

    std::vector<Row> columns;

    void set(Index3D i) { columns.at(i.x) &= (1u << i.y); }

    template <typename Op> void updateWith(OneSide const &other, Op op) {
        for (std::size_t i = 0; i < columns.size(); ++i) {
            auto &me = columns.at(i);
            me = op(me, other.columns.at(i));
        }
    }

    std::size_t count() const {
        return std::accumulate(
            columns.begin(), columns.end(), std::size_t{0},
            [](std::size_t sum, Row r) { return sum + __builtin_popcount(r); });
    }
};

} // namespace

template <> void puzzleA<2022, 18>(std::istream &input, std::ostream &output) {
    std::vector inputData(std::istream_iterator<Index3D>{input},
                          std::istream_iterator<Index3D>{});

    /*
    const auto mc = maxCoord(inputData);
    OneSide s(mc);
    for (Index3D i : inputData) {
        s.set(i);
    }
    output << s.count() << '\n';
    */

    std::size_t n = 0;
    for (int i = 0; i < 3; ++i) {
        std::sort(inputData.begin(), inputData.end());
        Index3D last{-1, -1, -1};
        for (Index3D &i : inputData) {
            if (last.x != i.x || last.y != i.y) {
                n += 2;
            } else if (last.z + 1 != i.z) {
                n += 2;
            }
            last = i;
            i.rot();
        }
    }
    output << n << '\n';
}

template <> void puzzleB<2022, 18>(std::istream &input, std::ostream &output) {}

} // namespace advent::common
