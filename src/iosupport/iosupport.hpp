#ifndef INCLUDED_ADVENT_IOSUPPORT_IOSUPPORT_HPP
#define INCLUDED_ADVENT_IOSUPPORT_IOSUPPORT_HPP

#include <filesystem>
#include <functional>
#include <iosfwd>
#include <string>
#include <vector>

namespace advent::iosupport {

void loadFileInLines(std::istream &stream,
                     std::function<void(std::string_view)> const &perLine);

auto loadFileInLines(std::istream &stream) -> std::vector<std::string>;

auto loadFileInLines(std::filesystem::path const &filePath)
    -> std::vector<std::string>;

struct ExpectChar {
    char expectedChar;

    void read(std::istream &stream) const;
    friend std::istream &operator>>(std::istream &stream, ExpectChar ec) {
        ec.read(stream);
        return stream;
    }
};

} // namespace advent::iosupport

#endif
