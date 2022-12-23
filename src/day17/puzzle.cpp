#include "adventofcode.hpp"

#include <algorithm>
#include <array>
#include <cstdint>
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

constexpr std::size_t spawnFree = 3;
constexpr std::size_t blockHeight = 4;
constexpr std::size_t spaceTotal = spawnFree + blockHeight;

enum class Dir : signed char { left, right };
std::istream &operator>>(std::istream &stream, Dir &dir) {
    char tmp;
    if (stream >> tmp) {
        if (tmp == '<') {
            dir = Dir::left;
        } else if (tmp == '>') {
            dir = Dir::right;
        } else {
            stream.putback(tmp);
            stream.setstate(std::ios_base::failbit);
        }
    }
    return stream;
}

using Jets = std::vector<Dir>;

/// @brief lower 7 bits used, the MSB is left, the LSB is right
using Field = std::uint8_t;
/// @brief Rows that define an item, higher indices are to the bottom of the item.
using Item = std::array<Field, blockHeight>;
using ItemU = std::uint32_t;

/// @brief All possible parts aligned to the bottom and two positions from the left.
constexpr std::array<Item, 5> items = {{
    {0b000'00000u, 0b000'00000u, 0b000'00000u, 0b000'11110u},
    {0b000'00000u, 0b000'01000u, 0b000'11100u, 0b000'01000u},
    {0b000'00000u, 0b000'00100u, 0b000'00100u, 0b000'11100u},
    {0b000'10000u, 0b000'10000u, 0b000'10000u, 0b000'10000u},
    {0b000'00000u, 0b000'00000u, 0b000'11000u, 0b000'11000u},
}};

struct ItemsRing {
    std::size_t itemIndex{};

    Item next() {
        Item i = items.at(itemIndex);
        itemIndex = (itemIndex + 1) % items.size();
        return i;
    }
};

bool shiftLeft(Field &f) {
    if (0b0'10'00000u & f) {
        return false;
    }
    f <<= 1;
    return true;
}
bool shiftRight(Field &f) {
    if (0b0'00'00001u & f) {
        return false;
    }
    f >>= 1;
    return true;
}
bool shiftLeft(Item &i) {
    Item c = i;
    bool ok = true;
    for (auto &f : c) {
        ok = ok && shiftLeft(f);
    }
    if (ok) i = c;
    //std::clog << "  << " << ok << '\n';
    return ok;
}
bool shiftRight(Item &i) {
    Item c = i;
    bool ok = true;
    for (auto &f : c) {
        ok = ok && shiftRight(f);
    }
    if (ok) i = c;
    //std::clog << "  >> " << ok << '\n';
    return ok;
}

template <typename I1, typename I2>
bool overlap(I1 i1, I2 i2) {
    for (int i = 0; i < blockHeight; ++i) {
        if (*i1 & *i2) return true;
        ++i1;
        ++i2;
    }
    return false;
}

struct JetsRing {
    Jets jets;
    std::size_t jetsIndex{};

    JetsRing(std::istream &input)
    : jets(std::istream_iterator<Dir>{input}, std::istream_iterator<Dir>{})
    {}

    Dir next() {
        Dir d = jets.at(jetsIndex);
        jetsIndex = (jetsIndex + 1) % jets.size();
        return d;
    }
};

void adjustFreeSpace(std::vector<Field> &vec) {
    auto first = vec.rbegin();
    auto last = vec.rend();
    int numFree{};
    for (int i = 0; i < spaceTotal; ++i) {
        if (first == last) break;
        if (*first != 0) break;
        ++numFree;
        ++first;
    }
    //std::clog << "Free space: " << numFree << '\n';
    vec.resize(vec.size() + spaceTotal - numFree);
}

template <typename I1, typename I2>
void manifest(I1 i1, I2 i2) {
    std::transform(i1, i1 + blockHeight, i2, i1, std::bit_or<Field>{});
}

} // namespace

template <> void puzzleA<2022, 17>(std::istream &input, std::ostream &output) {
    JetsRing jets(input);
    ItemsRing items{};

    std::vector<Field> tower;
    for (int i = 0; i < 2022; ++i) {
        adjustFreeSpace(tower);
        //std::clog << 'r' << tower.size() - spaceTotal << '\n';
        auto pos = tower.rbegin();
        auto bottom = tower.rend() - blockHeight;

        auto item = items.next();
        for (;;) {
            auto tmp = item;
            auto dir = jets.next();
            switch (dir) {
                case Dir::left: shiftLeft(tmp); break;
                case Dir::right: shiftRight(tmp); break;
            }
            if (!overlap(pos, tmp.begin())) {
                item = tmp;
            }
            if (pos == bottom) break;
            if (overlap(pos + 1, item.begin())) break;
            ++pos;
        }
        manifest(pos, item.begin());
        //std::clog << +tower.front() << '#' << std::distance(tower.begin(), pos.base()) << '\n';
        //std::clog << tower.size() - std::count(tower.begin(), tower.end(), 0u) << '\n';
    }
    adjustFreeSpace(tower);
    output << tower.size() - spaceTotal << '\n';
}

template <> void puzzleB<2022, 17>(std::istream &input, std::ostream &output) {}

} // namespace advent::common
