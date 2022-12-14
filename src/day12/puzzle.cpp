#include "adventofcode.hpp"

#include "iosupport/iosupport.hpp"

#include <boost/graph/breadth_first_search.hpp>
#include <boost/graph/dijkstra_shortest_paths.hpp>
#include <boost/graph/graph_concepts.hpp>
#include <boost/iterator/counting_iterator.hpp>
#include <boost/iterator/filter_iterator.hpp>
#include <boost/iterator/indirect_iterator.hpp>
#include <boost/property_map/property_map.hpp>

#include <algorithm>
#include <array>
#include <cassert>
#include <charconv>
#include <functional>
#include <iostream>
#include <istream>
#include <iterator>
#include <limits>
#include <numeric>
#include <ostream>
#include <queue>
#include <unordered_set>

namespace advent::common {

namespace {

struct Dim {
    std::ptrdiff_t columns{};
    std::ptrdiff_t rows{};
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
};

struct DimIterator
    : boost::iterator_facade<DimIterator, Index2D,
                             boost::single_pass_traversal_tag, Index2D> {

    DimIterator() = default;
    /// @brief Construct the begin iterator of any dimensions with the specified
    /// 'col' number of columns.
    /// @param cols Number of columns in the dimensions.
    DimIterator(std::ptrdiff_t cols) : columns{cols} {}
    /// @brief Construct the past-the-end iterator of the specified 'dim'.
    /// @param dim The dimensions to use.
    DimIterator(Dim dim) : columns{dim.columns}, pos{dim.rows, 0} {}

  private:
    friend class boost::iterator_core_access;

    std::ptrdiff_t columns{};
    Index2D pos{};

    void increment() {
        if (++pos.col >= columns) {
            pos.col = 0;
            ++pos.row;
        }
    }
    Index2D dereference() const { return pos; }
    bool equal(DimIterator const &other) const { return pos == other.pos; }
};

template <typename T> struct FieldT {
    std::vector<T> data;
    Dim dim;

    T &operator[](Index2D pos) {
        return data.at(pos.row * dim.columns + pos.col);
    }

    struct Map {
        FieldT *field;
        using key_type = Index2D;
        using value_type = T;
        using reference = T &;
        using category = boost::lvalue_property_map_tag;

        T &operator[](Index2D pos) {
            return (*field)[pos];
        }
        // T const &operator[](Index2D pos) const {
        //     return data.at(pos.row * dim.columns + pos.col);
        // }

        friend T &get(Map m, Index2D pos) { return m[pos]; }
        friend void put(Map m, Index2D pos, T const &value) { m[pos] = value; }
    };
    Map map() { return Map{this}; }
};

using Field = FieldT<char>;

char charSE(char c) {
    switch (c) {
    case 'S':
        return 'a';
    case 'E':
        return 'z';
    default:
        return c;
    }
}

bool isAdjacentChar(char src, char dst) {
    return charSE(dst) <= charSE(src) + 1;
}

struct HeightGraph {
    Field const *field{};

    struct traversal_category : boost::vertex_list_graph_tag,
                                boost::incidence_graph_tag {};
    using directed_category = boost::directed_tag;

    using vertex_descriptor = Index2D;

    using edge_descriptor = std::pair<vertex_descriptor, vertex_descriptor>;
    using edge_parallel_category = boost::disallow_parallel_edge_tag;

    struct AdjacencyIterator
        : boost::iterator_facade<AdjacencyIterator, edge_descriptor,
                                 boost::single_pass_traversal_tag,
                                 edge_descriptor> {
        Field const *field{};
        Index2D source;
        int neighborIndex{};

        AdjacencyIterator() = default;
        AdjacencyIterator(Field const *field)
            : field(field), neighborIndex(4) {}
        AdjacencyIterator(Field const *field, Index2D source)
            : field(field), source(source) {
            testAndNext();
        }

      private:
        friend class boost::iterator_core_access;

        char at(Index2D pos, int rowOffset = 0, int colOffset = 0) const {
            int row = pos.row + rowOffset;
            int col = pos.col + colOffset;
            int idx = field->dim.columns * row + col;
            return field->data.at(idx);
        }

        void testAndNext() {
            char self = at(source);
            for (; neighborIndex < 4; ++neighborIndex) {
                switch (neighborIndex) {
                case 0:
                    if (source.row == 0)
                        continue;
                    if (!isAdjacentChar(self, at(source, -1, 0)))
                        continue;
                    break;
                case 1:
                    if (source.col + 1 == field->dim.columns)
                        continue;
                    if (!isAdjacentChar(self, at(source, 0, 1)))
                        continue;
                    break;
                case 2:
                    if (source.row + 1 == field->dim.rows)
                        continue;
                    if (!isAdjacentChar(self, at(source, 1, 0)))
                        continue;
                    break;
                case 3:
                    if (source.col == 0)
                        continue;
                    if (!isAdjacentChar(self, at(source, 0, -1)))
                        continue;
                    break;
                }
                break;
            }
        }

        void increment() {
            ++neighborIndex;
            testAndNext();
        }
        bool equal(AdjacencyIterator const &other) const {
            return neighborIndex == other.neighborIndex;
        }
        edge_descriptor dereference() const {
            switch (neighborIndex) {
            case 0:
                return {source, {source.row - 1, source.col + 0}};
            case 1:
                return {source, {source.row + 0, source.col + 1}};
            case 2:
                return {source, {source.row + 1, source.col + 0}};
            case 3:
                return {source, {source.row + 0, source.col - 1}};
            }
            throw std::logic_error(
                "unexpected dereference for adjacency iterator");
        }
    };

    // VertexListGraph
    using vertex_iterator = DimIterator;
    using vertices_size_type = std::size_t;
    friend std::pair<vertex_iterator, vertex_iterator>
    vertices(HeightGraph const &g) {
        return std::pair(DimIterator(g.field->dim.columns),
                         DimIterator(g.field->dim));
    }
    friend vertices_size_type num_vertices(HeightGraph const &g) {
        return g.field->dim.columns * g.field->dim.rows;
    }

    // IncidenceGraph
    using out_edge_iterator = AdjacencyIterator;
    using degree_size_type = std::size_t;
    friend vertex_descriptor source(edge_descriptor e, HeightGraph const &) {
        return e.first;
    }
    friend vertex_descriptor target(edge_descriptor e, HeightGraph const &) {
        return e.second;
    }
    friend std::pair<out_edge_iterator, out_edge_iterator>
    out_edges(vertex_descriptor u, HeightGraph const &g) {
        return std::pair(AdjacencyIterator(g.field, u),
                         AdjacencyIterator(g.field));
    }
    friend degree_size_type out_degree(vertex_descriptor u,
                                       HeightGraph const &g) {
        auto [f, l] = out_edges(u, g);
        return std::distance(f, l);
    }

    // Other
    friend std::ptrdiff_t get(boost::vertex_index_t, HeightGraph const &g,
                              vertex_descriptor v) {
        return v.row * g.field->dim.columns + v.col;
    }

    static Index2D null_vertex() {
        return {std::numeric_limits<int>::max(),
                std::numeric_limits<int>::max()};
    }
};

BOOST_CONCEPT_ASSERT((boost::GraphConcept<HeightGraph>));

Field readField(std::istream &input) {
    Field field;
    iosupport::loadFileInLines(input, [&](std::string_view row) {
        if (field.dim.columns == 0) {
            field.dim.columns = row.size();
        } else if (field.dim.columns != row.size()) {
            throw std::runtime_error("Rows have different lengths");
        }
        field.data.insert(field.data.end(), row.begin(), row.end());
    });
    field.dim.rows = field.data.size() / field.dim.columns;
    return field;
}

Index2D fromLinear(Dim dim, std::ptrdiff_t index) {
    return Index2D{index / dim.columns, index % dim.columns};
}

std::ptrdiff_t findChar(std::vector<char> const &vec, char what) {
    const auto pos = std::find(vec.begin(), vec.end(), what);
    if (pos == vec.end()) {
        throw std::runtime_error("Did not find char: " + what);
    }
    return std::distance(vec.begin(), pos);
}

std::pair<Index2D, Index2D> findSE(Field const &field) {
    return {fromLinear(field.dim, findChar(field.data, 'S')),
            fromLinear(field.dim, findChar(field.data, 'E'))};
}

[[maybe_unused]] void printNeighbors(Field const &field, Index2D pos) {
    HeightGraph g{&field};
    std::clog << pos << ": " << std::flush;
    auto [f, l] = out_edges(pos, g);
    std::transform(f, l, std::ostream_iterator<Index2D>(std::clog, " "),
                   [](auto x) { return x.second; });
    std::clog << '\n';
}

struct BfsDistanceVisitor : public boost::default_bfs_visitor {
    FieldT<std::ptrdiff_t> &dmap;

    BfsDistanceVisitor(FieldT<std::ptrdiff_t> &dmap) : dmap(dmap) {}

    void examine_edge(HeightGraph::edge_descriptor e,
                      HeightGraph const &g) const {
        auto src = dmap[source(e, g)];
        auto &cur = dmap[target(e, g)];
        cur = std::min(src + 1, cur);
        //std::clog << "Examine edge: " << source(e, g) << " - " << target(e, g)
        //          << ": " << cur << std::endl;
    }
    void discover_vertex(HeightGraph::vertex_descriptor v,
                         HeightGraph const &g) const {
        //std::clog << "Discover vertex: " << v << ": " << dmap[v] << '\n';
    }
    void finish_vertex(HeightGraph::vertex_descriptor v,
                       HeightGraph const &g) const {
        //std::clog << "Finish vertex: " << v << ": " << dmap[v] << '\n';
    }
};

} // namespace

template <> void puzzleA<2022, 12>(std::istream &input, std::ostream &output) {
    Field field = readField(input);
    auto [vecS, vecE] = findSE(field);

    HeightGraph g{&field};

    FieldT<boost::default_color_type> colorMap;
    colorMap.dim = field.dim;
    colorMap.data.resize(field.data.size());

    FieldT<std::ptrdiff_t> distanceMap;
    distanceMap.dim = field.dim;
    distanceMap.data.resize(field.data.size(),
                            std::numeric_limits<std::ptrdiff_t>::max());
    distanceMap[vecS] = 0;

    BfsDistanceVisitor vis(distanceMap);
    boost::breadth_first_search(g, vecS,
                                boost::visitor(vis).color_map(colorMap.map()));
    output << distanceMap[vecE] << '\n';
}

template <> void puzzleB<2022, 12>(std::istream &input, std::ostream &output) {}

} // namespace advent::common
