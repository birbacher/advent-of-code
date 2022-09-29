#ifndef INCLUDED_ADVENT_IOSUPPORT_IOSUPPORT_HPP
#define INCLUDED_ADVENT_IOSUPPORT_IOSUPPORT_HPP

#include <filesystem>
#include <string>
#include <vector>
#include <iosfwd>
#include <functional>

namespace advent::iosupport {

void loadFileInLines(
        std::istream& stream,
        std::function<void (std::string_view)> const& perLine
    );

auto loadFileInLines(std::istream& stream)
    -> std::vector<std::string>;

auto loadFileInLines(std::filesystem::path const& filePath)
    -> std::vector<std::string>;

}

#endif
