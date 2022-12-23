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
/// @brief Rows that define an item, higher indices are to the bottom of the
/// item.
using Item = std::array<Field, blockHeight>;
/// @brief Rows that define an item, higher nibbles are to the bottom of the
/// item.
using ItemU = std::uint32_t;

constexpr ItemU getU(Item i) {
    return i.at(0) | (i.at(1) << 8) | (i.at(2) << 16) | (i.at(3) << 24);
}

/// @brief All possible parts aligned to the bottom and two positions from the
/// left.
constexpr std::array<ItemU, 5> items = {{
    {0b000'11110'000'00000'000'00000'000'00000u},
    {0b000'01000'000'11100'000'01000'000'00000u},
    {0b000'11100'000'00100'000'00100'000'00000u},
    {0b000'10000'000'10000'000'10000'000'10000u},
    {0b000'11000'000'11000'000'00000'000'00000u},
}};

struct ItemsRing {
    std::size_t itemIndex{};

    ItemU next() {
        ItemU i = items.at(itemIndex);
        itemIndex = (itemIndex + 1) % items.size();
        return i;
    }
};

void shiftLeft(ItemU &i, int n) {
    constexpr Field msb = 0b010'00000u; // 7bit highest bit
    constexpr ItemU mask = getU({msb, msb, msb, msb});
    if (0 == (i & mask)) {
        i <<= n;
    }
}
void shiftRight(ItemU &i, int n) {
    constexpr Field msb = 0b000'00001u; // 7bit lowest bit
    constexpr ItemU mask = getU({msb, msb, msb, msb});
    if (0 == (i & mask)) {
        i >>= n;
    }
}

ItemU bswp(ItemU u) {
    auto op = [u](int s, int l) { return ((u >> l) & 0xff) << s; };
    return op(24, 0) | op(16, 8) | op(8, 16) | op(0, 24);
}

ItemU getU(std::vector<Field>::const_reverse_iterator rit) {
    auto *const p = &*(rit.base() - 4);
    ItemU u;
    std::memcpy(&u, p, 4);
    return bswp(u);
}

void putU(std::vector<Field>::reverse_iterator rit, ItemU u) {
    auto *const p = &*(rit.base() - 4);
    u = bswp(u);
    std::memcpy(p, &u, 4);
}

bool overlap(std::vector<Field>::const_reverse_iterator rit, ItemU i) {
    return getU(rit) & i;
}
/*
template <typename I1, typename I2>
bool overlap(I1 i1, I2 i2) {
    for (int i = 0; i < blockHeight; ++i) {
        if (*i1 & *i2) return true;
        ++i1;
        ++i2;
    }
    return false;
}
*/
struct JetsRing {
    Jets jets;
    std::size_t jetsIndex{};

    JetsRing(std::istream &input)
        : jets(std::istream_iterator<Dir>{input},
               std::istream_iterator<Dir>{}) {}

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
        if (first == last)
            break;
        if (*first != 0)
            break;
        ++numFree;
        ++first;
    }
    // std::clog << "Free space: " << numFree << '\n';
    vec.resize(vec.size() + spaceTotal - numFree);
}

void adjustFreeSpace(std::vector<Field> &vec, std::size_t &counter) {
    adjustFreeSpace(vec);
    if (vec.size() > 10'000) {
        Field accu{};
        auto pos = vec.rbegin() + spaceTotal;
        for (;;) {
            if (pos == vec.rend())
                return;
            accu |= *pos;
            if (accu == 0b011'11111u)
                break;
            ++pos;
        }
        counter += std::distance(pos, vec.rend());
        vec.erase(vec.begin(), pos.base());
    }
}

void manifest(std::vector<Field>::reverse_iterator rit, ItemU i) {
    putU(rit, getU(rit) | i);
}

std::size_t computeHeight(JetsRing &jets, ItemsRing &items, std::size_t iters) {
    std::size_t counter = 0;
    std::vector<Field> tower;
    for (std::size_t i = 0; i < iters; ++i) {
        adjustFreeSpace(tower, counter);
        auto pos = tower.rbegin() + spawnFree;
        auto bottom = tower.rend() - blockHeight;

        auto item = items.next();
        for (int pre = 0; pre < spawnFree; ++pre) {
            auto dir = jets.next();
            shiftLeft(item, 1 ^ int(dir));
            shiftRight(item, int(dir));
        }
        for (;;) {
            auto tmp = item;
            auto dir = jets.next();
            shiftLeft(tmp, 1 ^ int(dir));
            shiftRight(tmp, int(dir));
            if (!overlap(pos, tmp)) {
                item = tmp;
            }
            if (pos == bottom)
                break;
            if (overlap(pos + 1, item))
                break;
            ++pos;
        }
        manifest(pos, item);

        // std::clog << +tower.front() << '#' << std::distance(tower.begin(),
        // pos.base()) << '\n'; std::clog << tower.size() -
        // std::count(tower.begin(), tower.end(), 0u) << '\n';
    }
    adjustFreeSpace(tower, counter);
    return counter + tower.size() - spaceTotal;
}

} // namespace

template <> void puzzleA<2022, 17>(std::istream &input, std::ostream &output) {
    JetsRing jets(input);
    ItemsRing items{};

    constexpr std::size_t iters = 2022;
    output << computeHeight(jets, items, iters) << '\n';
}

template <> void puzzleB<2022, 17>(std::istream &input, std::ostream &output) {
    JetsRing jets(input);
    ItemsRing items{};

    // constexpr std::size_t iters = 1'000'000'000'000; // 1 TB
    constexpr std::size_t iters   =       100'000'000;
    output << computeHeight(jets, items, iters) << '\n';
}

} // namespace advent::common
