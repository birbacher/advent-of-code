cmake_minimum_required (VERSION 3.20)
project (AdventOfCode2021)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED YES)

add_compile_options(-fsanitize=undefined)
add_compile_options(-fno-sanitize-recover)
add_link_options(-fsanitize=undefined)

add_subdirectory(src)
