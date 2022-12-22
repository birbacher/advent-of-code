#include "day16/puzzle.hpp"

#include <boost/graph/johnson_all_pairs_shortest.hpp>
#include <boost/property_map/property_map.hpp>

#include <cstdint>

namespace advent::common::day16 {

void loadState(State &s, std::vector<Line> const& lines) {
    auto mapId = get(ccid_t{}, s.g);
    auto mapRate = get(flow_rate_t{}, s.g);

    for (auto &line : lines) {
        const auto v = add_vertex(s.g);
        s.vs[line.id] = v;
        put(mapId, v, line.id);
        put(mapRate, v, line.flowRate);
        if (line.flowRate > 0) {
            s.valves.push_back(v);
        }
    }
    for (auto &line : lines) {
        const auto src = s.vs.at(line.id);
        for (auto target : line.edges) {
            add_edge(src, s.vs.at(target), s.g);
        }
    }
    s.src = s.vs.at("AA"_cc);

    auto V = num_vertices(s.g);
    s.dist.resize(V, std::vector<std::ptrdiff_t>(V));

    boost::static_property_map<std::ptrdiff_t> mapEdgeWeight{1};

    boost::johnson_all_pairs_shortest_paths(s.g, s.dist,
                                            boost::weight_map(mapEdgeWeight));
}

} // namespace advent::common
