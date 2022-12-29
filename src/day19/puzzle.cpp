#include "adventofcode.hpp"

#include "iosupport/iosupport.hpp"

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
#include <vector>

namespace advent::common {

namespace {

using iosupport::literals::operator""_e;

struct Blueprint {
    int oreRobotOres{};
    int clayRobotOres{};
    int obsidianRobotOres{};
    int obsidianRobotClays{};
    int geodeRobotOres{};
    int geodeRobotObsidians{};

    friend std::istream &operator>>(std::istream &stream, Blueprint &bl) {
        return stream >> "Each"_e >> "ore"_e >> "robot"_e >> "costs"_e >>
               bl.oreRobotOres >> "ore."_e >> "Each"_e >> "clay"_e >>
               "robot"_e >> "costs"_e >> bl.clayRobotOres >> "ore."_e >>
               "Each"_e >> "obsidian"_e >> "robot"_e >> "costs"_e >>
               bl.obsidianRobotOres >> "ore"_e >> "and"_e >>
               bl.obsidianRobotClays >> "clay."_e >> "Each"_e >> "geode"_e >>
               "robot"_e >> "costs"_e >> bl.geodeRobotOres >> "ore"_e >>
               "and"_e >> bl.geodeRobotObsidians >> "obsidian."_e;
    }
};

struct InputLine {
    int id{};
    Blueprint blueprint;

    friend std::istream &operator>>(std::istream &stream, InputLine &il) {
        return stream >> "Blueprint"_e >> il.id >> ':'_e >> il.blueprint;
    }
};

constexpr std::integral_constant<std::size_t, 0> kOre;
constexpr std::integral_constant<std::size_t, 1> kClay;
constexpr std::integral_constant<std::size_t, 2> kObsidian;
constexpr std::integral_constant<std::size_t, 3> kGeode;

using Rep = std::array<int, 4>;
void printRep(std::ostream &stream, Rep const &rep) {
    stream << '(' << std::get<0>(rep) << ',' << ' ' << std::get<1>(rep) << ','
           << ' ' << std::get<2>(rep) << ',' << ' ' << std::get<3>(rep) << ')';
}

struct Robots {
    Rep rep{};
    constexpr int &operator[](std::size_t i) { return rep[i]; }
    constexpr int operator[](std::size_t i) const { return rep[i]; }

    friend std::ostream &operator<<(std::ostream &stream, Robots const &r) {
        stream << "Robots";
        printRep(stream, r.rep);
        return stream;
    }
};

struct Resources {
    Rep rep{};
    constexpr int &operator[](std::size_t i) { return rep[i]; }
    constexpr int operator[](std::size_t i) const { return rep[i]; }

    friend std::ostream &operator<<(std::ostream &stream, Resources const &r) {
        stream << "Resources";
        printRep(stream, r.rep);
        return stream;
    }
};

void produceResources(Resources &res, Robots const &robots) {
    for (int i = 0; i < 4; ++i) {
        res[i] += robots[i];
    }
}

void produceUntil(Resources &res, Robots const &robots, std::size_t kind,
                  int target, int &remTime) {
    if (res[kind] >= target) {
        // std::clog << "kind: " << kind << " res: " << res[kind]
        //           << " target: " << target << '\n';
        return;
    }
    int delta = target - res[kind] + robots[kind] - 1;
    int rounds = delta / robots[kind];
    // std::clog << "kind: " << kind << " res: " << res[kind]
    //           << " target: " << target << " rounds: " << rounds
    //           << " robots: " << robots[kind]
    //           << " remTime: " << (remTime - rounds) << '\n';
    for (std::size_t k = 0; k < 4; ++k) {
        res[k] += rounds * robots[k];
    }
    remTime -= rounds;
    /*
    while (res[kind] < target) {
        produceResources(res, robots);
        --remTime;
    }
    */
}

void produceOreRobot(Resources &res, Robots &robots, Blueprint const &bl) {
    res[kOre] -= bl.oreRobotOres;
    ++robots[kOre];
}

void produceClayRobot(Resources &res, Robots &robots, Blueprint const &bl) {
    res[kOre] -= bl.clayRobotOres;
    ++robots[kClay];
}

void produceObsidianRobot(Resources &res, Robots &robots, Blueprint const &bl) {
    res[kOre] -= bl.obsidianRobotOres;
    res[kClay] -= bl.obsidianRobotClays;
    ++robots[kObsidian];
}

void produceGeodeRobot(Resources &res, Robots &robots, Blueprint const &bl) {
    res[kOre] -= bl.geodeRobotOres;
    res[kObsidian] -= bl.geodeRobotObsidians;
    ++robots[kGeode];
}

struct BlueprintInfo {
    Blueprint bl;
    int maxOreNeeded;
};

BlueprintInfo makeInfo(Blueprint const &bl) {
    BlueprintInfo info{bl};
    for (auto x : {bl.oreRobotOres, bl.clayRobotOres, bl.obsidianRobotOres,
                   bl.geodeRobotOres}) {
        info.maxOreNeeded = std::max(info.maxOreNeeded, x);
    }
    return info;
}

constexpr Robots initialRobots{{1, 0, 0, 0}};

struct State {
    BlueprintInfo info;
    int bestSolution{};
    int minRoundsGeode{};

    void recordResult(int result) {
        if (result > bestSolution) {
            bestSolution = result;
            updateMinRoundsGeode();
            std::clog << "best: " << bestSolution
                      << " min rounds: " << minRoundsGeode << std::endl;
        }
    }

    void updateMinRoundsGeode() {
        int rounds{};
        int robots{};
        int sum{};
        while (sum < bestSolution) {
            sum += robots;
            ++robots;
            ++rounds;
        }
        minRoundsGeode = rounds;
    }
};

void newSearch(State &state, Robots const &robots, Resources const &res,
               const int remainingMinutes) {
    if (remainingMinutes == 0) {
        state.recordResult(res[kGeode]);
    } else if (robots[kGeode] == 0 && state.minRoundsGeode > remainingMinutes) {
        return;
    } else if (remainingMinutes > 0) {
        Blueprint const &bl = state.info.bl;

        if (robots[kGeode] == 0 && state.minRoundsGeode == remainingMinutes) {
            goto justGeode;
        }

        // Try creating an ore robot next:
        if (robots[kOre] < state.info.maxOreNeeded) {
            int tmpMin = remainingMinutes;
            Resources tmpRes = res;
            produceUntil(tmpRes, robots, kOre, bl.oreRobotOres, tmpMin);
            if (tmpMin >= 1) {
                Robots tmpRobots = robots;
                produceOreRobot(tmpRes, tmpRobots, bl);
                --tmpMin;
                produceResources(tmpRes, robots);
                newSearch(state, tmpRobots, tmpRes, tmpMin);
            }
        }

        // Try creating a clay robot next:
        if (robots[kClay] < bl.obsidianRobotClays &&
            remainingMinutes * robots[kClay] + res[kClay] <
                remainingMinutes * bl.obsidianRobotClays) {
            int tmpMin = remainingMinutes;
            Resources tmpRes = res;
            produceUntil(tmpRes, robots, kOre, bl.clayRobotOres, tmpMin);
            if (tmpMin >= 1) {
                Robots tmpRobots = robots;
                produceClayRobot(tmpRes, tmpRobots, bl);
                --tmpMin;
                produceResources(tmpRes, robots);
                newSearch(state, tmpRobots, tmpRes, tmpMin);
            }
        }

        // Try creating an obsidian robot next:
        if (robots[kClay] > 0 && robots[kObsidian] < bl.geodeRobotObsidians) {
            int tmpMin = remainingMinutes;
            Resources tmpRes = res;
            produceUntil(tmpRes, robots, kOre, bl.obsidianRobotOres, tmpMin);
            produceUntil(tmpRes, robots, kClay, bl.obsidianRobotClays, tmpMin);
            if (tmpMin >= 1) {
                Robots tmpRobots = robots;
                produceObsidianRobot(tmpRes, tmpRobots, bl);
                --tmpMin;
                produceResources(tmpRes, robots);
                newSearch(state, tmpRobots, tmpRes, tmpMin);
            }
        }

        // Try creating a geode robot next:
    justGeode:
        if (robots[kObsidian] > 0) {
            int tmpMin = remainingMinutes;
            Resources tmpRes = res;
            produceUntil(tmpRes, robots, kOre, bl.geodeRobotOres, tmpMin);
            produceUntil(tmpRes, robots, kObsidian, bl.geodeRobotObsidians,
                         tmpMin);
            if (tmpMin >= 1) {
                Robots tmpRobots = robots;
                produceGeodeRobot(tmpRes, tmpRobots, bl);
                --tmpMin;
                produceResources(tmpRes, robots);
                newSearch(state, tmpRobots, tmpRes, tmpMin);
            }
        }

        // Otherwise, try producing resources for the remaining time:
        {
            int extraGeodes = remainingMinutes * robots[kGeode];
            state.recordResult(res[kGeode] + extraGeodes);
        }
    }
}

} // namespace

template <> void puzzleA<2022, 19>(std::istream &input, std::ostream &output) {
    std::vector lines(std::istream_iterator<InputLine>{input},
                      std::istream_iterator<InputLine>{});

    int result{};
    for (auto [id, bl] : lines) {
        State state{makeInfo(bl)};
        Robots robots = initialRobots;
        Resources res;
        newSearch(state, robots, res, 24);
        const int geodes = state.bestSolution;
        std::clog << id << ": " << geodes << std::endl;
        result += id * geodes;
    }
    output << result << '\n';

    // Wrong answer 1716 is too low.
}

template <> void puzzleB<2022, 19>(std::istream &input, std::ostream &output) {
    std::vector lines(std::istream_iterator<InputLine>{input},
                      std::istream_iterator<InputLine>{});

    if (lines.size() > 3) {
        lines.resize(3);
    }

    int result{1};
    for (auto [id, bl] : lines) {
        State state{makeInfo(bl)};
        Robots robots = initialRobots;
        Resources res;
        newSearch(state, robots, res, 32);
        std::clog << id << ": " << state.bestSolution << std::endl;
        result *= state.bestSolution;
    }
    output << result << '\n';
}

} // namespace advent::common
