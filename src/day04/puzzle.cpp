#include "adventofcode.hpp"

#include "iosupport/iosupport.hpp"

#include <algorithm>
#include <istream>
#include <iterator>
#include <numeric>
#include <ostream>
#include <stdexcept>
#include <string>

namespace advent::common {

namespace {

struct SectionRange {
    int first{}, last{};
};

/** Inclusive range denoted by the lowest and highest section numbers,
 * both non-negatve.
 */
void ensureValidSectionRange(SectionRange rng) {
    if (rng.first >= 0 && rng.last >= 0 && rng.first <= rng.last) {
        return;
    }

    throw std::runtime_error("Invalid section range");
}

std::istream &operator>>(std::istream &stream, SectionRange &rng) {
    char dashIgnored;
    stream >> rng.first >> dashIgnored >> rng.last;
    ensureValidSectionRange(rng);
    return stream;
}

struct ElfPair {
    SectionRange fstElf, sndElf;
};

std::istream &operator>>(std::istream &stream, ElfPair &p) {
    char commaIgnored;
    return stream >> p.fstElf >> commaIgnored >> p.sndElf;
}

bool isSecondContainedInFirst(SectionRange const &lhs,
                              SectionRange const &rhs) {
    return lhs.first <= rhs.first && lhs.last >= rhs.last;
}
bool isOneContainedInOther(ElfPair const &p) {
    return isSecondContainedInFirst(p.fstElf, p.sndElf) ||
           isSecondContainedInFirst(p.sndElf, p.fstElf);
}

} // namespace

template <> void puzzleA<2022, 4>(std::istream &input, std::ostream &output) {
    output << std::count_if(std::istream_iterator<ElfPair>(input),
                            std::istream_iterator<ElfPair>(),
                            &isOneContainedInOther)
           << '\n';
}

template <> void puzzleB<2022, 4>(std::istream &input, std::ostream &output) {}

} // namespace advent::common
