add_executable(advent main.cpp)
target_include_directories(advent PRIVATE .)
add_subdirectory(iosupport)
add_subdirectory(day01)
