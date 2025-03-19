#pragma once

#include "puzzle.hpp"
#include <vector>
#include <memory>

// Represents a path through the puzzle
struct Path {
    std::vector<std::pair<int, int>> positions;
    std::vector<int> directions;
};

class Solver {
public:
    explicit Solver(std::unique_ptr<Puzzle> p);
    
    // Main solving methods
    std::vector<Path> solve();
    
    // Set maximum number of solutions to find (0 for unlimited)
    void setMaxSolutions(int max) { maxSolutions = max; }
    
private:
    std::unique_ptr<Puzzle> puzzle;
    std::vector<Path> solutions;
    int maxSolutions = 0;
    
    // Helper methods
    void solveFromStart(int startX, int startY, int numEndpoints);
    void solveLoop(int x, int y, int numEndpoints, Path& path);
    bool validatePath(const Path& path);
    std::vector<std::pair<int, int>> findStartPoints();
    int countEndpoints();
}; 