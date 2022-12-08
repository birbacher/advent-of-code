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

} // namespace

template <> void puzzleA<2022, 8>(std::istream &input, std::ostream &output) {
    Field field = readField(input);
    std::vector<bool> selected(field.dim.columns * field.dim.rows);
    scan(field, selected);
    output << std::count(selected.begin(), selected.end(), true) << '\n';
}

template <> void puzzleB<2022, 8>(std::istream &input, std::ostream &output) {}

} // namespace advent::common
