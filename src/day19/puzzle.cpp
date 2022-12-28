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

struct Robots {
    int numOreRobots{};
    int numClayRobots{};
    int numObsidianRobots{};
    int numGeodeRobots{};
};

struct Resources {
    int numOre{};
    int numClay{};
    int numObsidian{};
    int numGeodes{};
};

void produceResources(Resources &res, Robots const &robots) {
    res.numOre += robots.numOreRobots;
    res.numClay += robots.numClayRobots;
    res.numObsidian += robots.numObsidianRobots;
    res.numGeodes += robots.numGeodeRobots;
}

void produceOreRobot(Resources &res, Robots &robots, Blueprint const &bl) {
    res.numOre -= bl.oreRobotOres;
    ++robots.numOreRobots;
}

void produceClayRobot(Resources &res, Robots &robots, Blueprint const &bl) {
    res.numOre -= bl.clayRobotOres;
    ++robots.numClayRobots;
}

void produceObsidianRobot(Resources &res, Robots &robots, Blueprint const &bl) {
    res.numOre -= bl.obsidianRobotOres;
    res.numClay -= bl.obsidianRobotClays;
    ++robots.numObsidianRobots;
}

void produceGeodeRobot(Resources &res, Robots &robots, Blueprint const &bl) {
    res.numOre -= bl.geodeRobotOres;
    res.numObsidian -= bl.geodeRobotObsidians;
    ++robots.numGeodeRobots;
}

constexpr Robots initialRobots{1, 0, 0, 0};

int search(Blueprint const &bl, Robots const &robots, Resources const &res,
           const int remainingMinutes) {
    if (remainingMinutes == 0) {
        return res.numGeodes;
    }
    if (remainingMinutes < 0) {
        return 0;
    }

    int result = res.numGeodes;

    // Try creating an ore robot next:
    {
        int tmpMin = remainingMinutes;
        Resources tmpRes = res;
        while (tmpRes.numOre < bl.oreRobotOres) {
            --tmpMin;
            produceResources(tmpRes, robots);
        }
        Robots tmpRobots = robots;
        produceOreRobot(tmpRes, tmpRobots, bl);
        --tmpMin;
        produceResources(tmpRes, robots);
        result = std::max(result, search(bl, tmpRobots, tmpRes, tmpMin));
    }

    // Try creating a clay robot next:
    {
        int tmpMin = remainingMinutes;
        Resources tmpRes = res;
        while (tmpRes.numOre < bl.clayRobotOres) {
            --tmpMin;
            produceResources(tmpRes, robots);
        }
        Robots tmpRobots = robots;
        produceClayRobot(tmpRes, tmpRobots, bl);
        --tmpMin;
        produceResources(tmpRes, robots);
        result = std::max(result, search(bl, tmpRobots, tmpRes, tmpMin));
    }

    // Try creating an obsidian robot next:
    if (robots.numClayRobots > 0) {
        int tmpMin = remainingMinutes;
        Resources tmpRes = res;
        while (tmpRes.numOre < bl.obsidianRobotOres) {
            --tmpMin;
            produceResources(tmpRes, robots);
        }
        while (tmpRes.numClay < bl.obsidianRobotClays) {
            --tmpMin;
            produceResources(tmpRes, robots);
        }
        Robots tmpRobots = robots;
        produceObsidianRobot(tmpRes, tmpRobots, bl);
        --tmpMin;
        produceResources(tmpRes, robots);
        result = std::max(result, search(bl, tmpRobots, tmpRes, tmpMin));
    }

    // Try creating a geode robot next:
    if (robots.numObsidianRobots > 0) {
        int tmpMin = remainingMinutes;
        Resources tmpRes = res;
        while (tmpRes.numOre < bl.geodeRobotOres) {
            --tmpMin;
            produceResources(tmpRes, robots);
        }
        while (tmpRes.numObsidian < bl.geodeRobotObsidians) {
            --tmpMin;
            produceResources(tmpRes, robots);
        }
        Robots tmpRobots = robots;
        produceGeodeRobot(tmpRes, tmpRobots, bl);
        --tmpMin;
        produceResources(tmpRes, robots);
        result = std::max(result, search(bl, tmpRobots, tmpRes, tmpMin));
    }

    // Otherwise, try producing resources for the remaining time:
    if (result == 0) {
        int tmpMin = remainingMinutes;
        Resources tmpRes = res;
        while (tmpMin) {
            --tmpMin;
            produceResources(tmpRes, robots);
        }
        return tmpRes.numGeodes;
    }

    return result;
}

} // namespace

template <> void puzzleA<2022, 19>(std::istream &input, std::ostream &output) {
    std::vector lines(std::istream_iterator<InputLine>{input},
                      std::istream_iterator<InputLine>{});

    int result{};
    for (auto [id, bl] : lines) {
        Robots robots = initialRobots;
        Resources res;
        const int geodes = search(bl, robots, res, 24);
        std::clog << id << ": " << geodes << std::endl;
        result += id * geodes;
    }
    output << result << '\n';

    // Wrong answer 1716 is too low.
}

template <> void puzzleB<2022, 19>(std::istream &input, std::ostream &output) {}

} // namespace advent::common
