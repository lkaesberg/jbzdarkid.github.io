cmake_minimum_required(VERSION 3.10)
project(PuzzleSolver)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Include FetchContent for downloading dependencies
include(FetchContent)

# Fetch nlohmann_json
FetchContent_Declare(json
    URL https://github.com/nlohmann/json/releases/download/v3.11.2/json.tar.xz
)
FetchContent_MakeAvailable(json)

# Add source files
add_executable(puzzle_solver
    main.cpp
    puzzle.cpp
    solver.cpp
    polyomino.cpp
)

# Link against nlohmann_json
target_link_libraries(puzzle_solver PRIVATE nlohmann_json::nlohmann_json) 