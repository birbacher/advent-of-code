#include "iosupport/iosupport.hpp"

#include <fstream>
#include <istream>
#include <iterator>
#include <stdexcept>

namespace advent::iosupport {

void loadFileInLines(std::istream &stream,
                     std::function<void(std::string_view)> const &perLine) {
    std::string line;
    while (std::getline(stream, line)) {
        perLine(line);
    }
    if (stream.fail() && !stream.eof())
        throw std::runtime_error("Could not fully read the input file");
}

auto loadFileInLines(std::istream &stream) -> std::vector<std::string> {
    std::vector<std::string> result;
    loadFileInLines(
        stream, [&result](std::string_view sv) { result.emplace_back(sv); });
    return result;
}

auto loadFileInLines(std::filesystem::path const &filePath)
    -> std::vector<std::string> {
    std::ifstream file(filePath);
    if (!file)
        throw std::runtime_error("Could not open input file");

    return loadFileInLines(file);
}

void ExpectChar::read(std::istream &stream) const {
    char tmp;
    if (stream >> tmp) {
        if (tmp != this->expectedChar) {
            stream.putback(tmp);
            stream.setstate(std::ios_base::failbit);
        }
    }
}

ExpectString ExpectString::operator[](char newDelim) && {
    this->delim = newDelim;
    return std::move(*this);
}

void ExpectString::read(std::istream &stream) const {
    if (!stream) {
        return;
    }
    std::string tmp;
    if (this->delim != 0) {
        stream >> std::ws;
        std::getline(stream, tmp, this->delim);
    }

    else {
        stream >> tmp;
    }
    if (stream) {
        if (tmp != this->expectedString) {
            stream.setstate(std::ios_base::failbit);
        }
    }
}

namespace literals {

ExpectString operator""_e(char const *cstr, std::size_t len) {
    return {{cstr, len}};
}

} // namespace literals
} // namespace advent::iosupport
