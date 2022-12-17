#include "adventofcode.hpp"

#include <algorithm>
#include <cassert>
#include <charconv>
#include <istream>
#include <iterator>
#include <ostream>
#include <string>
#include <vector>

namespace advent::common {

namespace {

struct Pair {
    std::string lft, rgt;

    friend std::istream &operator>>(std::istream &stream, Pair &p) {
        return stream >> p.lft >> p.rgt;
    }
};

struct CmpState {
    std::ptrdiff_t idxLft{}, idxRgt{};
};

void wrapInListAt(std::string &str, std::ptrdiff_t idx) {
    assert(0 <= idx);
    assert(idx < str.size());
    str.insert(str.begin() + idx, '[');
    for (;;) {
        ++idx;
        if (idx >= str.size()) {
            break;
        }
        if (!std::isdigit(str.at(idx))) {
            break;
        }
    }
    str.insert(str.begin() + idx, ']');
}

int consumeNum(std::string const &str, std::ptrdiff_t &idx) {
    const auto first = str.data() + idx;
    for (; idx != str.size(); ++idx) {
        if (!std::isdigit(str.at(idx))) {
            break;
        }
    }
    const auto last = str.data() + idx;
    int num{};
    auto [r, ec] = std::from_chars(first, last, num);
    if (ec != std::errc{}) {
        throw ec;
    }
    return num;
}

bool isOrdered(Pair &p, CmpState &state) {
beginning:
    if (state.idxRgt == p.rgt.size()) {
        return false;
    }
    if (state.idxLft == p.lft.size()) {
        return true;
    }
    const char cLft = p.lft.at(state.idxLft);
    const char cRgt = p.rgt.at(state.idxRgt);

    using namespace std::literals;

    if (cLft == '[') {
        if (cRgt == '[') {
            goto advance;
        }
        if (cRgt == ']') {
            return false;
        }
        if (cRgt == ',') {
            throw std::runtime_error("Invalid [ and , situation");
        }
        if (std::isdigit(cRgt)) {
            wrapInListAt(p.rgt, state.idxRgt);
            goto beginning;
        }
        throw std::runtime_error("Invalid character rgt: '"s + cRgt + '\'');
    } else if (cLft == ']') {
        if (cRgt == ']') {
            goto advance;
        }
        if (cRgt == '[') {
            return true;
        }
        if (cRgt == ',') {
            return true;
        }
        if (std::isdigit(cRgt)) {
            return true;
        }
        throw std::runtime_error("Invalid character rgt: '"s + cRgt + '\'');
    } else if (cLft == ',') {
        if (cRgt == ',') {
            goto advance;
        }
        if (cRgt == '[') {
            throw std::runtime_error("Invalid , and [ situation");
        }
        if (cRgt == ']') {
            return false;
        }
        if (std::isdigit(cRgt)) {
            throw std::runtime_error("Invalid , and digit situation");
        }
        throw std::runtime_error("Invalid character rgt: '"s + cRgt + '\'');
    } else if (std::isdigit(cLft)) {
        if (cRgt == '[') {
            wrapInListAt(p.lft, state.idxLft);
            goto beginning;
        }
        if (cRgt == ']') {
            return false;
        }
        if (cRgt == ',') {
            throw std::runtime_error("Invalid digit and , situation");
        }
        if (std::isdigit(cRgt)) {
            // two numbers
            int l = consumeNum(p.lft, state.idxLft);
            int r = consumeNum(p.rgt, state.idxRgt);
            if (l < r)
                return true;
            if (r < l)
                return false;
            goto beginning;
        }
        throw std::runtime_error("Invalid character rgt: '"s + cRgt + '\'');
    }
    throw std::runtime_error("Invalid character lft: '"s + cLft + '\'');
advance:
    ++state.idxLft;
    ++state.idxRgt;
    goto beginning;
}

struct Order {
    bool operator()(std::string const &lhs, std::string const &rhs) const {
        Pair p{lhs, rhs};
        CmpState state;
        return isOrdered(p, state);
    }
};

} // namespace

template <> void puzzleA<2022, 13>(std::istream &input, std::ostream &output) {
    int result = 0;
    int index = 0;
    Pair p;
    while (input >> p) {
        ++index;
        CmpState state;
        const bool b = isOrdered(p, state);
        if (b) {
            result += index;
        }
    }
    output << result << '\n';
}

template <> void puzzleB<2022, 13>(std::istream &input, std::ostream &output) {
    const std::string div1 = "[[2]]";
    const std::string div2 = "[[6]]";
    std::vector<std::string> packets{div1, div2};
    std::copy(std::istream_iterator<std::string>(input),
              std::istream_iterator<std::string>(),
              std::back_inserter(packets));
    std::sort(packets.begin(), packets.end(), Order{});
    auto p1 = std::find(packets.begin(), packets.end(), div1);
    auto p2 = std::find(p1, packets.end(), div2);
    auto f1 = 1 + std::distance(packets.begin(), p1);
    auto f2 = 1 + std::distance(packets.begin(), p2);
    output << f1 * f2 << '\n';
}

} // namespace advent::common
