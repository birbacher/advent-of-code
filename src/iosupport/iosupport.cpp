#include "iosupport/iosupport.hpp"

#include <istream>
#include <fstream>
#include <iterator>
#include <stdexcept>

namespace advent::iosupport {

void loadFileInLines(
        std::istream& stream,
        std::function<void (std::string_view)> const& perLine
    )
{
    std::string line;
    while (std::getline(stream, line)) {
        perLine(line);
    }
    if (stream.fail() && !stream.eof())
        throw std::runtime_error("Could not fully read the input file");
}

auto loadFileInLines(std::istream& stream)
    -> std::vector<std::string>
{
    std::vector<std::string> result;
    loadFileInLines(stream, [&result](std::string_view sv) { result.emplace_back(sv); });
    return result;
}

auto loadFileInLines(std::filesystem::path const& filePath)
    -> std::vector<std::string>
{
    std::ifstream file(filePath);
    if (!file) throw std::runtime_error("Could not open input file");

    return loadFileInLines(file);
}

}
