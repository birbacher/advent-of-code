#include "day16/puzzle.hpp"
#include "adventofcode.hpp"

#include <boost/property_map/property_map.hpp>

#include <algorithm>
#include <cassert>
#include <iostream>
#include <ostream>
#include <string>

namespace advent::common {
namespace day16 {
namespace {

std::ptrdiff_t getFlow(State const &s, std::ptrdiff_t remainingTime,
                       std::vector<Graph::vertex_descriptor> const &route) {
    std::ptrdiff_t result{};
    Graph::vertex_descriptor last = route.at(0);
    std::ptrdiff_t travelled = 0;
    auto mapId = get(ccid_t{}, s.g);
    auto mapRate = get(flow_rate_t{}, s.g);
    for (auto cur : route) {
        if (last != cur) {
            // std::clog << "Travel " << s.dist[last][cur];
            travelled += s.dist[last][cur];
            ++travelled;
            // std::clog << '\n';
            if (travelled >= remainingTime) {
                break;
            }
            auto rate = get(mapRate, cur);
            // std::clog << ", at " << get(mapId, cur) << ", rate " << rate <<
            // ':';
            auto factor = remainingTime - travelled;
            // std::clog << " factor " << factor << "mins";
            result += rate * factor;
            last = cur;
            // std::clog << '\n';
        }
    }
    return result;
}

struct IterState {
    std::vector<bool> used;
    std::vector<Graph::vertex_descriptor> order;
    std::ptrdiff_t result{};

    IterState(std::size_t n) : used(n, false) { order.reserve(n); }

    void run(Graph::vertex_descriptor from, State &state, std::ptrdiff_t rem) {
        bool someFound = false;
        if (rem > 0) {
            for (std::size_t i = 0; i < state.valves.size(); ++i) {
                if (used.at(i))
                    continue;
                someFound = true;
                used.at(i) = true;
                auto next = state.valves.at(i);
                order.push_back(next);
                run(next, state, rem - 1 - state.dist[from][next]);
                order.pop_back();
                used.at(i) = false;
            }
        }
        if (someFound == false) {
            auto mapId = get(ccid_t{}, state.g);
            auto f = getFlow(state, 30, order);
            result = std::max(result, f);
        }
    }

    void run2(Graph::vertex_descriptor from, State &s, std::ptrdiff_t rem,
              std::ptrdiff_t sum) {
        auto mapRate = get(flow_rate_t{}, s.g);

        bool someFound = false;
        if (rem > 0) {
            for (std::size_t i = 0; i < s.valves.size(); ++i) {
                if (used.at(i))
                    continue;
                used.at(i) = true;
                someFound = true;

                auto next = s.valves.at(i);
                //order.push_back(next);

                auto distance = s.dist[from][next];
                auto nextRem = rem - 1 - distance;
                run2(next, s, nextRem, sum + get(mapRate, next) * nextRem);

                //order.pop_back();
                used.at(i) = false;
            }
        }
        if (someFound == false) {
            //auto f = getFlow(s, 30, order);
            result = std::max(result, sum);
        }
    }
};

} // namespace
} // namespace day16

using namespace day16;

template <> void puzzleA<2022, 16>(std::istream &input, std::ostream &output) {
    std::vector lines = readInput(input);

    State s;
    loadState(s, lines);

    /*
    std::vector route = {
        s.vs.at("AA"_cc), s.vs.at("DD"_cc), s.vs.at("BB"_cc), s.vs.at("JJ"_cc),
        s.vs.at("HH"_cc), s.vs.at("EE"_cc), s.vs.at("CC"_cc),
    };

    output << getFlow(s, 30, route) << '\n';
    */

    IterState is(s.valves.size());
    is.run2(s.src, s, 30, 0);
    output << is.result << '\n';
}

template <> void puzzleB<2022, 16>(std::istream &input, std::ostream &output) {}

} // namespace advent::common
