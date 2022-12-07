#include "adventofcode.hpp"

#include "iosupport/iosupport.hpp"

#include <algorithm>
#include <array>
#include <cassert>
#include <charconv>
#include <functional>
#include <iostream>
#include <istream>
#include <iterator>
#include <map>
#include <numeric>
#include <ostream>
#include <set>

namespace advent::common {

namespace {

struct DirEntry {
    DirEntry *oneUp{};
    std::size_t myTotalSize{};
    std::map<std::string, DirEntry> subdirs;
    std::map<std::string, std::size_t> files;
};

struct Builder {
    DirEntry *const root;
    DirEntry *current = root;

    void changeDir(std::string const &dir) {
        if (dir == "/") {
            current = root;
        } else if (dir == "..") {
            current = current->oneUp;
        } else {
            const auto pos = current->subdirs.find(dir);
            if (pos == current->subdirs.end()) {
                throw std::runtime_error("Subdir not found: " + dir);
            }
            current = &(pos->second);
        }
    }
    void listDir(std::string const &name) {
        current->subdirs[name].oneUp = current;
    }
    void listFile(std::size_t size, std::string const &name) {
        current->files[name] = size;
    }
};

std::size_t
computeSizeAndReport(DirEntry &node,
                     std::function<void(DirEntry const &)> const &report10k) {
    node.myTotalSize = 0;
    for (auto [name, subdir] : node.subdirs) {
        node.myTotalSize += computeSizeAndReport(subdir, report10k);
    }
    for (auto [name, size] : node.files) {
        node.myTotalSize += size;
    }
    if (node.myTotalSize <= 100'000) {
        report10k(node);
    }
    return node.myTotalSize;
}

} // namespace

template <> void puzzleA<2022, 7>(std::istream &input, std::ostream &output) {
    DirEntry root;
    Builder builder{&root};
    std::string tmp, tmp2;
    while (input >> tmp) {
        if (tmp == "dir" && (input >> tmp)) {
            builder.listDir(tmp);
        } else if (tmp == "$" && (input >> tmp)) {
            if (tmp == "ls") {

            } else if (tmp == "cd" && (input >> tmp)) {
                builder.changeDir(tmp);
            } else {
                throw std::runtime_error("Unknown command: " + tmp);
            }
        } else {
            std::size_t size;
            const std::string_view sv = tmp;
            auto [ptr, ec] = std::from_chars(sv.begin(), sv.end(), size);
            if (ec != std::errc())
                throw ec;
            if (input >> tmp) {
                builder.listFile(size, tmp);
            }
        }
    }
    std::size_t sum10k = 0;
    computeSizeAndReport(root, [&](DirEntry const &reported) {
        sum10k += reported.myTotalSize;
    });
    output << sum10k << '\n';
}

template <> void puzzleB<2022, 7>(std::istream &input, std::ostream &output) {}

} // namespace advent::common
