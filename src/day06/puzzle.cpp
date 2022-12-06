#include "adventofcode.hpp"

#include "iosupport/iosupport.hpp"

#include <algorithm>
#include <array>
#include <cassert>
#include <functional>
#include <iostream>
#include <istream>
#include <iterator>
#include <numeric>
#include <ostream>
#include <queue>

namespace advent::common {

namespace {

constexpr std::string_view abc = "abcdefghijklmnopqrstuvwxyz";
using Abc = std::array<int, abc.size()>;

std::size_t getCharIndex(char c) {
    auto pos = std::find(abc.begin(), abc.end(), c);
    if (pos == abc.end()) {
        using namespace std::literals;
        throw std::domain_error(
            "Given character is not valid for 'getCharIndex': '"s + c + "'");
    }
    return std::distance(abc.begin(), pos);
}

constexpr std::size_t targetWindowSize = 4;

} // namespace

template <> void puzzleA<2022, 6>(std::istream &input, std::ostream &output) {
    Abc state{};
    int numDifferentChars = 0;
    int resultingIndex = 0;
    std::queue<int> window;
    std::for_each(std::istream_iterator<char>(input),
                  std::istream_iterator<char>(), [&](char c) {
                      {
                          const std::size_t i = getCharIndex(c);
                          int &count = state.at(i);
                          if (count == 0) {
                              ++numDifferentChars;
                          }
                          ++count;
                          window.push(i);
                      }
                      ++resultingIndex;

                      if (window.size() > targetWindowSize) {
                          int remove = window.front();
                          window.pop();
                          int &count = state.at(remove);
                          assert(count > 0);
                          --count;
                          if (count == 0) {
                              --numDifferentChars;
                          }
                      }

                      if (numDifferentChars == targetWindowSize) {
                          output << resultingIndex << '\n';
                          input.setstate(
                              input.rdstate() |
                              std::ios_base::failbit); // terminate input
                      }
                  });
}

template <> void puzzleB<2022, 6>(std::istream &input, std::ostream &output) {}

} // namespace advent::common
