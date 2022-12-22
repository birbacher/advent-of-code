#ifndef INCLUDED_ADVENTOFCODE_DAY16_PUZZLE_HPP
#define INCLUDED_ADVENTOFCODE_DAY16_PUZZLE_HPP

#include <boost/graph/properties.hpp>
#include <boost/graph/adjacency_list.hpp>

#include <iosfwd>
#include <unordered_map>
#include <vector>

namespace advent::common::day16 {

struct CharCharId {
    char c1{}, c2{};

    constexpr auto p() const { return std::pair(c1, c2); }

    friend constexpr bool operator==(CharCharId lhs, CharCharId rhs) {
        return lhs.p() == rhs.p();
    }
    friend constexpr bool operator<(CharCharId lhs, CharCharId rhs) {
        return lhs.p() < rhs.p();
    }
};

std::istream &operator>>(std::istream &stream, CharCharId &id);
std::ostream &operator<<(std::ostream &stream, CharCharId id);

constexpr CharCharId operator""_cc(char const *str, std::size_t n) {
    if (n != 2)
        throw std::logic_error("need length 2");
    return {str[0], str[1]};
}

} // namespace advent::common::day16

template <> struct std::hash<advent::common::day16::CharCharId> {
    std::size_t operator()(advent::common::day16::CharCharId id) const {
        return std::hash<int>()(id.c1 * 256 + id.c2);
    }
};

namespace advent::common::day16 {

struct ccid_t {
    typedef boost::vertex_property_tag kind;
};

struct flow_rate_t {
    typedef boost::vertex_property_tag kind;
};

using VertexProperties =
    boost::property<ccid_t, CharCharId, boost::property<flow_rate_t, int>>;

using Graph = boost::adjacency_list<boost::vecS,        // OutEdgeList
                                    boost::vecS,        // VertexList
                                    boost::undirectedS, // Directed
                                    VertexProperties,   // VertexProperties
                                    boost::no_property, // EdgeProperties
                                    boost::no_property, // GraphProperties
                                    boost::vecS         // EdgeList
                                    >;

struct State {
    Graph g;
    std::unordered_map<CharCharId, Graph::vertex_descriptor> vs;
    Graph::vertex_descriptor src;
    std::vector<Graph::vertex_descriptor> valves;
    std::vector<std::vector<std::ptrdiff_t>> dist;
};

struct Line {
    CharCharId id;
    int flowRate;
    std::vector<CharCharId> edges;
};

std::istream &operator>>(std::istream &stream, Line &line);

std::vector<Line> readInput(std::istream&stream);

void loadState(State &s, std::vector<Line> const& lines);

} // namespace advent::common::day16

#endif
