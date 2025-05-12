cmake_minimum_required(VERSION 3.10)
project(GuardAndTowers)

find_package(Python3 REQUIRED COMPONENTS Interpreter Development)
find_package(pybind11 REQUIRED)


# Create Python module target
pybind11_add_module(pybind_module pybind.cpp)

# Link to Python
target_link_libraries(pybind_module PRIVATE Python3::Python)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Include pybind11 if installed via Homebrew
include_directories(/opt/homebrew/include)
link_directories(/opt/homebrew/lib)

# Your main C++ executable
add_executable(main
    main.cpp
    board.cpp
    calculations.cpp
    benchmarking.cpp
)

add_executable(BoardTests
    board_tests.cpp
    board.cpp
    calculations.cpp
    benchmarking.cpp
)


target_include_directories(pybind_module PRIVATE /opt/homebrew/include)
target_link_libraries(pybind_module PRIVATE /opt/homebrew/lib)