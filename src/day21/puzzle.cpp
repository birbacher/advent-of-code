#include "adventofcode.hpp"

#include <boost/graph/directed_graph.hpp>
#include <boost/graph/topological_sort.hpp>

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
#include <tuple>
#include <variant>
#include <vector>

namespace advent::common {

namespace {

using Num = std::int64_t;

enum Operation : char {
    OpAdd,
    OpSub,
    OpMul,
    OpDiv,
};

std::istream &operator>>(std::istream &stream, Operation &op) {
    char tmp;
    if (stream >> tmp) {
        switch (tmp) {
        case '+':
            op = OpAdd;
            break;
        case '-':
            op = OpSub;
            break;
        case '*':
            op = OpMul;
            break;
        case '/':
            op = OpDiv;
            break;
        default:
            stream.putback(tmp);
            stream.setstate(std::ios_base::failbit);
        }
    }
    return stream;
}

Num runOp(Num lhs, Operation op, Num rhs) {
    switch (op) {
    case OpAdd:
        return lhs + rhs;
    case OpSub:
        return lhs - rhs;
    case OpMul:
        return lhs * rhs;
    case OpDiv:
        return lhs / rhs;
    }
    throw std::runtime_error("Unknown operation in runOp");
}

struct BinOp {
    std::string lhs, rhs;
    Operation op;
};

struct Line {
    std::string name;
    std::variant<Num, BinOp> job;

    friend std::istream &operator>>(std::istream &stream, Line &line) {
        stream >> std::ws;
        std::getline(stream, line.name, ':');
        std::string tmp;
        if (stream >> tmp) {
            std::string_view sv = tmp;
            auto [ptr, ec] =
                std::from_chars(sv.begin(), sv.end(), line.job.emplace<Num>());
            if (ec != std::errc()) {
                auto &binOp = line.job.emplace<BinOp>();
                binOp.lhs = std::move(tmp);
                stream >> binOp.op >> binOp.rhs;
            }
        }
        return stream;
    }
};

using Graph = boost::directed_graph<>;

struct State {
    Graph g;
    std::unordered_map<std::string, Graph::vertex_descriptor> vertexByName;
    std::vector<Graph::vertex_descriptor> topoSorted;

    void read(std::vector<Line> const &lines) {
        for (auto const &line : lines) {
            vertexByName[line.name] = g.add_vertex();
        }
        for (auto const &line : lines) {
            struct Vis {
                State *state;
                Graph::vertex_descriptor src;
                void operator()(Num num) const {};
                void operator()(BinOp const &binOp) const {
                    state->g.add_edge(src, state->vertexByName.at(binOp.lhs));
                    state->g.add_edge(src, state->vertexByName.at(binOp.rhs));
                }
            } vis{this, vertexByName.at(line.name)};
            std::visit(vis, line.job);
        }
    }

    void generateTopoSorted() {
        topoSorted.resize(num_vertices(g));
        boost::topological_sort(g, topoSorted.begin());
    }
};

} // namespace

template <> void puzzleA<2022, 21>(std::istream &input, std::ostream &output) {
    std::vector lines(std::istream_iterator<Line>{input},
                      std::istream_iterator<Line>{});

    State state;
    state.read(lines);
    state.generateTopoSorted();
    std::vector<Num> results(num_vertices(state.g));
    auto m = get(boost::vertex_index, state.g);
    for (auto vd : state.topoSorted) {
        auto i = get(m, vd);
        auto const &job = lines.at(i).job;
        if (Num const *const num = std::get_if<Num>(&job)) {
            results.at(i) = *num;
        } else if (BinOp const *const binOp = std::get_if<BinOp>(&job)) {
            auto iLhs = get(m, state.vertexByName.at(binOp->lhs));
            auto iRhs = get(m, state.vertexByName.at(binOp->rhs));
            results.at(i) =
                runOp(results.at(iLhs), binOp->op, results.at(iRhs));
        } else {
            throw std::logic_error("Unknown choice in variant or empty");
        }
    }
    output << results.at(get(m, state.vertexByName.at("root"))) << '\n';
}

template <> void puzzleB<2022, 21>(std::istream &input, std::ostream &output) {}

} // namespace advent::common
