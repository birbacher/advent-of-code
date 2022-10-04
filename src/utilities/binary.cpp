#include "utilities/binary.hpp"

#include <limits>
#include <numeric>
#include <stdexcept>

namespace utilities::binary {

int safeDouble(int v) {
    if (v > std::numeric_limits<int>::max() / 2) {
        throw std::domain_error("binary rep too large");
    }
    return 2 * v;
}

int accumBinary(int accu, char ch) { return safeDouble(accu) + int(ch == '1'); }

int readBinary(std::string_view sv) {
    return std::accumulate(sv.begin(), sv.end(), 0, &accumBinary);
}

} // namespace utilities::binary
