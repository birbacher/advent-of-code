#include "day02/puzzle.hpp"

#include <algorithm>
#include <numeric>
#include <iterator>
#include <ostream>
#include <istream>
#include <stdexcept>
#include <string>

namespace advent::day02 {

namespace {

enum RPS {
    Rock,
    Paper,
    Scissors,
};

RPS recognize(char encoded) {
    switch (encoded) {
        case 'A': case 'X': return Rock;
        case 'B': case 'Y': return Paper;
        case 'C': case 'Z': return Scissors;
    }
    using namespace std::literals;
    throw std::runtime_error("Invalid encoded RPS character: '"s + encoded + "'");
}

struct Round {
    RPS opponent;
    RPS ownMove;
};

std::istream& operator>>(std::istream& stream, Round& round) {
    char other, own;
    if (stream >> other >> own) {
        round.opponent = recognize(other);
        round.ownMove = recognize(own);
    }
    return stream;
}

int scoreOwnShape(Round round) {
    switch (round.ownMove) {
        case Rock: return 1;
        case Paper: return 2;
        case Scissors: return 3;
    }
    throw std::logic_error("Invalid RPS value");
}

int normalizedDifference(RPS lhs, RPS rhs) {
    int valLhs = static_cast<int>(lhs);
    int valRhs = static_cast<int>(rhs);
    return (valLhs - valRhs + 3) % 3;
}

int scoreWin(Round round) {
    switch (normalizedDifference(round.opponent, round.ownMove)) {
        case 0: return 3; // Draw
        case 1: return 0; // We lost
        case 2: return 6; // We won
    }
    throw std::logic_error("Invalid normalized difference");
}

int scoreRound(Round round) {
    return scoreOwnShape(round) + scoreWin(round);
}

}

void puzzleA(std::istream& input, std::ostream& output)
{
    output << std::accumulate(
        std::istream_iterator<Round>(input),
        std::istream_iterator<Round>(),
        0,
        [](int sum, Round round) {
            return sum + scoreRound(round);
        }
    ) << '\n';
}

void puzzleB(std::istream& input, std::ostream& output)
{
}

}
