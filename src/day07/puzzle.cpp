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
#include <limits>

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
                     std::function<void(DirEntry const &)> const &report) {
    node.myTotalSize = 0;
    for (auto [name, subdir] : node.subdirs) {
        node.myTotalSize += computeSizeAndReport(subdir, report);
    }
    for (auto [name, size] : node.files) {
        node.myTotalSize += size;
    }
    report(node);
    return node.myTotalSize;
}

void readTree(DirEntry &root, std::istream &input) {
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
}

constexpr std::size_t sizeOfDisk = 70'000'000;
constexpr std::size_t spaceRequired = 30'000'000;

std::size_t computeRequiredDelete(DirEntry const &root) {
    const std::size_t currentlyFree = sizeOfDisk - root.myTotalSize;
    if (spaceRequired <= currentlyFree) {
        throw std::runtime_error("Already enough space is free");
    }
    const std::size_t missingFree = spaceRequired - currentlyFree;
    return missingFree;
}

} // namespace

template <> void puzzleA<2022, 7>(std::istream &input, std::ostream &output) {
    DirEntry root;
    readTree(root, input);

    std::size_t sum100k = 0;
    computeSizeAndReport(root, [&](DirEntry const &reported) {
        if (reported.myTotalSize <= 100'000) {
            sum100k += reported.myTotalSize;
        }
    });
    output << sum100k << '\n';
}

template <> void puzzleB<2022, 7>(std::istream &input, std::ostream &output) {
    DirEntry root;
    readTree(root, input);

    computeSizeAndReport(root, [&](DirEntry const &reported) {});

    const std::size_t requiredDelete = computeRequiredDelete(root);

    std::size_t trackSmallest = std::numeric_limits<std::size_t>::max();
    computeSizeAndReport(root, [&](DirEntry const &reported) {
        if (reported.myTotalSize >= requiredDelete) {
            trackSmallest = std::min(trackSmallest, reported.myTotalSize);
        }
    });

    output << trackSmallest << '\n';
}

} // namespace advent::common
