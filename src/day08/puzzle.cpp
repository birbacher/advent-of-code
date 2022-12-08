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
#include <set>

namespace advent::common {

namespace {

struct Dim {
    std::ptrdiff_t columns{};
    std::ptrdiff_t rows{};
};

struct Access {
    std::ptrdiff_t offset{};
    std::ptrdiff_t stride{};
    std::ptrdiff_t numElements{};

    std::ptrdiff_t operator[](std::ptrdiff_t index) const {
        return offset + stride * index;
    }
};

Access accessRow(Dim dim, std::ptrdiff_t row) {
    return Access{dim.columns * row, 1, dim.columns};
}
Access accessRowReverse(Dim dim, std::ptrdiff_t row) {
    return Access{dim.columns * (row + 1) - 1, -1, dim.columns};
}
Access accessCol(Dim dim, std::ptrdiff_t col) {
    return Access{col, dim.columns, dim.rows};
}
Access accessColReverse(Dim dim, std::ptrdiff_t col) {
    return Access{dim.columns * (dim.rows - 1) + col, -dim.columns, dim.rows};
}

using Mapping = Access(Dim dim, std::ptrdiff_t row);

struct Field {
    std::string data;
    Dim dim;
};

void scan(Access const &access, Field const &field,
          std::vector<bool> &results) {
    char c = 0;
    for (std::ptrdiff_t i = 0; i < access.numElements; ++i) {
        char d = field.data.at(access[i]);
        if (d <= c)
            continue;
        results.at(access[i]) = true;
        c = d;
    }
}

void scan(Mapping *mapping, Field const &field, std::vector<bool> &results) {
    for (std::ptrdiff_t row = 0; row < field.dim.rows; ++row) {
        Access access = mapping(field.dim, row);
        scan(access, field, results);
    }
}

void scan(Field const &field, std::vector<bool> &results) {
    scan(&accessRow, field, results);
    scan(&accessRowReverse, field, results);
    scan(&accessCol, field, results);
    scan(&accessColReverse, field, results);
}

Field readField(std::istream &input) {
    Field field;
    iosupport::loadFileInLines(input, [&](std::string_view row) {
        if (field.dim.columns == 0) {
            field.dim.columns = row.size();
        } else if (field.dim.columns != row.size()) {
            throw std::runtime_error("Rows have different lengths");
        }
        field.data += row;
    });
    field.dim.rows = field.data.size() / field.dim.columns;
    return field;
}

Access advance(Access access, std::ptrdiff_t n) {
    return Access{access[n], access.stride, access.numElements - n};
}

Access accessRight(Dim dim, std::ptrdiff_t row, std::ptrdiff_t col) {
    return advance(accessRow(dim, row), col);
}
Access accessLeft(Dim dim, std::ptrdiff_t row, std::ptrdiff_t col) {
    return advance(accessRowReverse(dim, row), dim.columns - col - 1);
}
Access accessDown(Dim dim, std::ptrdiff_t row, std::ptrdiff_t col) {
    return advance(accessCol(dim, col), row);
}
Access accessUp(Dim dim, std::ptrdiff_t row, std::ptrdiff_t col) {
    return advance(accessColReverse(dim, col), dim.rows - row - 1);
}

int scenic(Field const &field, Access access) {
    const char me = field.data.at(access[0]);
    int count = 0;
    for (std::ptrdiff_t i = 1; i < access.numElements; ++i) {
        ++count;
        if (field.data.at(access[i]) >= me)
            break;
    }
    return count;
}

int scenic(Field const &field, std::ptrdiff_t row, std::ptrdiff_t col) {
    int score = 1;
    score *= scenic(field, accessRight(field.dim, row, col));
    score *= scenic(field, accessLeft(field.dim, row, col));
    score *= scenic(field, accessDown(field.dim, row, col));
    score *= scenic(field, accessUp(field.dim, row, col));
    return score;
}

int scenic(Field const &field) {
    int maxi = 0;
    // Skip outermost trees, because they have score zero.
    for (std::ptrdiff_t row = 1; row < field.dim.rows - 1; ++row) {
        for (std::ptrdiff_t col = 1; col < field.dim.columns - 1; ++col) {
            maxi = std::max(maxi, scenic(field, row, col));
        }
    }
    return maxi;
}

} // namespace

template <> void puzzleA<2022, 8>(std::istream &input, std::ostream &output) {
    Field field = readField(input);
    std::vector<bool> selected(field.dim.columns * field.dim.rows);
    scan(field, selected);
    output << std::count(selected.begin(), selected.end(), true) << '\n';
}

template <> void puzzleB<2022, 8>(std::istream &input, std::ostream &output) {
    Field field = readField(input);
    output << scenic(field) << '\n';
}

} // namespace advent::common
