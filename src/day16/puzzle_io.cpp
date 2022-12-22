#include "day16/puzzle.hpp"

#include <algorithm>
#include <cassert>
#include <charconv>
#include <istream>
#include <iterator>
#include <ostream>

namespace advent::common::day16 {

std::istream &operator>>(std::istream &stream, CharCharId &id) {
    return stream >> id.c1 >> id.c2;
}
std::ostream &operator<<(std::ostream &stream, CharCharId id) {
    return stream << id.c1 << id.c2;
}

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

struct ExpectString {
    std::string str;
    char delim{};
    friend std::istream &operator>>(std::istream &stream,
                                    ExpectString const &es) {
        if (!stream) {
            return stream;
        }
        std::string tmp;
        if (es.delim != 0) {
            stream >> std::ws;
            std::getline(stream, tmp, es.delim);
        }

        else {
            stream >> tmp;
        }
        if (stream) {
            if (tmp != es.str) {
                stream.setstate(std::ios_base::failbit);
            }
        }
        return stream;
    }
};

ExpectString operator""_e(char const *cstr, std::size_t len) {
    return {{cstr, len}};
}

} // namespace

std::istream &operator>>(std::istream &stream, Line &line) {
    // Some lines use "tunnels lead" and others "tunnel leads".
    // Just ignore both.  Same for "valves" and "valve".
    std::string tmp;
    stream >> "Valve"_e >> line.id >> "has"_e >> "flow"_e >>
        ExpectString{"rate", '='} >> line.flowRate >> ';'_e >> tmp >> tmp >>
        "to"_e >> tmp;
    line.edges.clear();
    for (; stream;) {
        CharCharId id;
        char tmp;
        if (stream >> id) {
            // Only include edges in one direction:
            if (line.id < id) {
                line.edges.push_back(id);
            }
            stream >> tmp;
            if (!stream) {
                // If we failed due to EOF, clear the error:
                if (stream.eof()) {
                    stream.clear(std::ios_base::eofbit);
                }
                break;
            }
            if (tmp != ',') {
                stream.putback(tmp);
                break;
            }
        }
    }
    return stream;
}

std::vector<Line> readInput(std::istream &stream) {
    return std::vector(std::istream_iterator<Line>{stream},
                       std::istream_iterator<Line>{});
}

} // namespace advent::common::day16
