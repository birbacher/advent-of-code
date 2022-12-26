#include "adventofcode.hpp"

#include <boost/graph/breadth_first_search.hpp>
#include <boost/graph/dijkstra_shortest_paths.hpp>
#include <boost/graph/graph_concepts.hpp>
#include <boost/iterator/zip_iterator.hpp>
#include <boost/property_map/property_map.hpp>
#include <boost/tuple/tuple.hpp>

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
    friend bool operator==(Index3D lhs, Index3D rhs) {
        return lhs.p() == rhs.p();
    }
    friend bool operator!=(Index3D lhs, Index3D rhs) {
        return lhs.p() != rhs.p();
    }

    friend std::ostream &operator<<(std::ostream &stream, Index3D i) {
        return stream << '(' << i.x << ',' << i.y << ',' << i.z << ')';
    }
    friend std::istream &operator>>(std::istream &stream, Index3D &i) {
        return stream >> i.x >> ','_e >> i.y >> ','_e >> i.z;
    }

    friend Index3D operator+(Index3D lhs, Index3D rhs) {
        return {lhs.x + rhs.x, lhs.y + rhs.y, lhs.z + rhs.z};
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

struct Dim3 {
    int x{};
    int y{};
    int z{};

    int volume() const { return x * y * z; }

    std::ptrdiff_t index(Index3D pos) const {
        return (pos.z * y + pos.y) * x + pos.x;
    }
};

struct DimIterator
    : boost::iterator_facade<DimIterator, Index3D,
                             boost::single_pass_traversal_tag, Index3D> {

    DimIterator() = default;
    /// @brief Construct the begin iterator of any dimensions with the specified
    /// 'col' number of columns and 'rows' number of rows.
    /// @param x Number of columns in the dimensions.
    /// @param y Number of rows in the dimensions.
    DimIterator(int x, int y) : x{x}, y{y} {}
    /// @brief Construct the past-the-end iterator of the specified 'dim'.
    /// @param dim The dimensions to use.
    DimIterator(Dim3 dim) : x{dim.x}, y{dim.y}, pos{0, 0, dim.z} {}

  private:
    friend class boost::iterator_core_access;

    int x{};
    int y{};
    Index3D pos{};

    void increment() {
        if (++pos.x >= x) {
            pos.x = 0;
            if (++pos.y >= y) {
                pos.y = 0;
                ++pos.z;
            }
        }
    }
    Index3D dereference() const { return pos; }
    bool equal(DimIterator const &other) const { return pos == other.pos; }
};

template <typename T> struct FieldT {
    std::vector<T> data;
    Dim3 dim;

    FieldT(Dim3 dim) : data(dim.volume()), dim(dim) {}
    FieldT(Dim3 dim, T const &t) : data(dim.volume(), t), dim(dim) {}

    T &operator[](Index3D pos) { return data.at(dim.index(pos)); }
    const T &operator[](Index3D pos) const { return data.at(dim.index(pos)); }

    struct Map {
        FieldT *field;
        using key_type = Index3D;
        using value_type = T;
        using reference = T &;
        using category = boost::lvalue_property_map_tag;

        T &operator[](Index3D pos) { return (*field)[pos]; }
        // T const &operator[](Index3D pos) const {
        //     return data.at(pos.row * dim.columns + pos.col);
        // }

        friend T &get(Map m, Index3D pos) { return m[pos]; }
        friend void put(Map m, Index3D pos, T const &value) { m[pos] = value; }
    };
    Map map() { return Map{this}; }
};

using Field = FieldT<signed char>;

struct HeightGraph {
    Field const *field{};

    struct traversal_category : boost::vertex_list_graph_tag,
                                boost::incidence_graph_tag {};
    using directed_category = boost::directed_tag;

    using vertex_descriptor = Index3D;

    using edge_descriptor = std::pair<vertex_descriptor, vertex_descriptor>;
    using edge_parallel_category = boost::disallow_parallel_edge_tag;

    struct AdjacencyIterator
        : boost::iterator_facade<AdjacencyIterator, edge_descriptor,
                                 boost::single_pass_traversal_tag,
                                 edge_descriptor> {
        Field const *field{};
        Index3D source;
        int neighborIndex{};

        AdjacencyIterator() = default;
        AdjacencyIterator(Field const *field)
            : field(field), neighborIndex(6) {}
        AdjacencyIterator(Field const *field, Index3D source)
            : field(field), source(source) {
            testAndNext();
        }

      private:
        friend class boost::iterator_core_access;

        char at(Index3D pos) const { return (*field)[pos]; }

        static constexpr std::array<Index3D, 6> offsets = {{
            {-1, 0, 0},
            {1, 0, 0},
            {0, -1, 0},
            {0, 1, 0},
            {0, 0, -1},
            {0, 0, 1},
        }};

        void testAndNext() {
            char self = at(source);
            for (; neighborIndex < 6; ++neighborIndex) {
                const Index3D pos = source + offsets[neighborIndex];
                switch (neighborIndex) {
                case 0:
                    if (pos.x <= 0)
                        continue;
                    break;
                case 1:
                    if (pos.x >= field->dim.x)
                        continue;
                    break;
                case 2:
                    if (pos.y <= 0)
                        continue;
                    break;
                case 3:
                    if (pos.y >= field->dim.y)
                        continue;
                    break;
                case 4:
                    if (pos.z <= 0)
                        continue;
                    break;
                case 5:
                    if (pos.z >= field->dim.z)
                        continue;
                    break;
                }
                if (at(pos))
                    continue;
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
            return {source, source + offsets.at(neighborIndex)};
        }
    };

    // VertexListGraph
    using vertex_iterator = DimIterator;
    using vertices_size_type = std::size_t;
    friend std::pair<vertex_iterator, vertex_iterator>
    vertices(HeightGraph const &g) {
        return std::pair(DimIterator(g.field->dim.x, g.field->dim.y),
                         DimIterator(g.field->dim));
    }
    friend vertices_size_type num_vertices(HeightGraph const &g) {
        return g.field->dim.volume();
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
        return g.field->dim.index(v);
    }

    static Index3D null_vertex() {
        return {std::numeric_limits<int>::max(),
                std::numeric_limits<int>::max(),
                std::numeric_limits<int>::max()};
    }
};

BOOST_CONCEPT_ASSERT((boost::GraphConcept<HeightGraph>));

FieldT<boost::default_color_type> loadMap(Field &field,
                                          HeightGraph::vertex_descriptor vecS) {
    FieldT<boost::default_color_type> colorMap(field.dim);

    HeightGraph g{&field};
    boost::default_bfs_visitor vis;
    boost::breadth_first_search(g, vecS,
                                boost::visitor(vis).color_map(colorMap.map()));

    return colorMap;
}

} // namespace

template <> void puzzleA<2022, 18>(std::istream &input, std::ostream &output) {
    std::vector inputData(std::istream_iterator<Index3D>{input},
                          std::istream_iterator<Index3D>{});

    /*
    const auto mc = maxCoord(inputData);
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

template <> void puzzleB<2022, 18>(std::istream &input, std::ostream &output) {
    std::vector inputData(std::istream_iterator<Index3D>{input},
                          std::istream_iterator<Index3D>{});
    auto mc = maxCoord(inputData);
    ++mc.x;
    ++mc.y;
    ++mc.z;
    Field f({mc.x, mc.y, mc.z});
    for (Index3D i : inputData) {
        f[i] = 1;
    }

    auto color = loadMap(f, Index3D{0, 0, 0});

    std::size_t n = 0;
    for (int x = 0; x < f.dim.x; ++x) {
        for (int y = 0; y < f.dim.y; ++y) {
            bool free = true;
            for (int z = 0; z < f.dim.z; ++z) {
                bool thisFree = (color[{x, y, z}] == boost::black_color);
                n += free != thisFree;
                free = thisFree;
            }
            if (!free)
                ++n;
        }
    }
    for (int z = 0; z < f.dim.z; ++z) {
        for (int x = 0; x < f.dim.x; ++x) {
            bool free = true;
            for (int y = 0; y < f.dim.y; ++y) {
                bool thisFree = (color[{x, y, z}] == boost::black_color);
                n += free != thisFree;
                free = thisFree;
            }
            if (!free)
                ++n;
        }
    }
    for (int y = 0; y < f.dim.y; ++y) {
        for (int z = 0; z < f.dim.z; ++z) {
            bool free = true;
            for (int x = 0; x < f.dim.x; ++x) {
                bool thisFree = (color[{x, y, z}] == boost::black_color);
                n += free != thisFree;
                free = thisFree;
            }
            if (!free)
                ++n;
        }
    }
    output << n << '\n';
    // Answer 2048 is too low.
}

} // namespace advent::common
