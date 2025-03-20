#include "solver.hpp"
#include <iostream>

Solver::Solver(std::unique_ptr<Puzzle> p) : puzzle(std::move(p)) {
    std::cout << "Created solver" << std::endl;
}

std::vector<Path> Solver::solve() {
    solutions.clear();
    
    // Find all start points
    auto startPoints = findStartPoints();
    if (startPoints.empty()) {
        std::cerr << "No start points found in puzzle" << std::endl;
        return solutions;
    }
    
    // Count total endpoints
    int numEndpoints = countEndpoints();
    if (numEndpoints == 0) {
        std::cerr << "No endpoints found in puzzle" << std::endl;
        return solutions;
    }
    
    // Try solving from each start point
    for (const auto& [startX, startY] : startPoints) {
        solveFromStart(startX, startY, numEndpoints);
        if (maxSolutions > 0 && solutions.size() >= maxSolutions) {
            break;
        }
    }
    
    return solutions;
}

void Solver::solveFromStart(int startX, int startY, int numEndpoints) {
    std::cout << "Starting solve from " << startX << "," << startY << std::endl;
    Path path;
    path.positions.push_back({startX, startY});
    path.directions.push_back(PATH_NONE);
    
    puzzle->clearLines();
    
    if (auto cell = puzzle->getCell(startX, startY)) {
        std::cout << "Got start cell" << std::endl;
        cell->line = LINE_BLACK;
        solveLoop(startX, startY, numEndpoints, path);
    } else {
        std::cout << "Failed to get start cell!" << std::endl;
    }
}

void Solver::solveLoop(int x, int y, int numEndpoints, Path& path) {
    if (maxSolutions > 0 && solutions.size() >= maxSolutions) {
        return;
    }
    
    auto cell = puzzle->getCell(x, y);
    if (!cell) {
        return;
    }
    
    // Don't check line status for the current cell since we just set it
    if (cell->gap > GAP_NONE) {
        return;
    }
    
    if (!cell->end.empty()) {
        // When we reach any endpoint, consider it a valid solution if the path is valid
        if (validatePath(path)) {
            solutions.push_back(path);
            // Don't return here - continue searching for more solutions
        }
    }
    
    // Try moving in each direction
    // We can move horizontally from even y coordinates
    if (y % 2 == 0) {
        // Try left
        if (x > 0) {
            if (auto nextCell = puzzle->getCell(x - 1, y)) {
                if (nextCell->line == LINE_NONE && nextCell->gap <= GAP_NONE) {
                    nextCell->line = LINE_BLACK;
                    path.directions.push_back(PATH_LEFT);
                    path.positions.push_back({x - 1, y});
                    solveLoop(x - 1, y, numEndpoints, path);
                    path.positions.pop_back();
                    path.directions.pop_back();
                    nextCell->line = LINE_NONE;
                }
            }
        }
        
        // Try right
        if (x < puzzle->getActualWidth() - 1) {
            if (auto nextCell = puzzle->getCell(x + 1, y)) {
                if (nextCell->line == LINE_NONE && nextCell->gap <= GAP_NONE) {
                    nextCell->line = LINE_BLACK;
                    path.directions.push_back(PATH_RIGHT);
                    path.positions.push_back({x + 1, y});
                    solveLoop(x + 1, y, numEndpoints, path);
                    path.positions.pop_back();
                    path.directions.pop_back();
                    nextCell->line = LINE_NONE;
                }
            }
        }
    }
    
    // We can move vertically from even x coordinates
    if (x % 2 == 0) {
        // Try up
        if (y > 0) {
            if (auto nextCell = puzzle->getCell(x, y - 1)) {
                if (nextCell->line == LINE_NONE && nextCell->gap <= GAP_NONE) {
                    nextCell->line = LINE_BLACK;
                    path.directions.push_back(PATH_TOP);
                    path.positions.push_back({x, y - 1});
                    solveLoop(x, y - 1, numEndpoints, path);
                    path.positions.pop_back();
                    path.directions.pop_back();
                    nextCell->line = LINE_NONE;
                }
            }
        }
        
        // Try down
        if (y < puzzle->getActualHeight() - 1) {
            if (auto nextCell = puzzle->getCell(x, y + 1)) {
                if (nextCell->line == LINE_NONE && nextCell->gap <= GAP_NONE) {
                    nextCell->line = LINE_BLACK;
                    path.directions.push_back(PATH_BOTTOM);
                    path.positions.push_back({x, y + 1});
                    solveLoop(x, y + 1, numEndpoints, path);
                    path.positions.pop_back();
                    path.directions.pop_back();
                    nextCell->line = LINE_NONE;
                }
            }
        }
    }
}

std::vector<std::pair<int, int>> Solver::findStartPoints() {
    std::cout << "Finding start points..." << std::endl;
    std::vector<std::pair<int, int>> startPoints;
    
    // Use actual grid dimensions
    for (int x = 0; x < puzzle->getActualWidth(); x++) {
        for (int y = 0; y < puzzle->getActualHeight(); y++) {
            if (auto cell = puzzle->getCell(x, y)) {
                if (cell->start) {
                    std::cout << "Found start at " << x << "," << y << std::endl;
                    startPoints.push_back({x, y});
                }
            }
        }
    }
    
    return startPoints;
}

int Solver::countEndpoints() {
    int numEndpoints = 0;
    std::cout << "Counting endpoints in puzzle..." << std::endl;
    
    // Get actual grid dimensions using the new getter methods
    int actualWidth = puzzle->getActualWidth();
    int actualHeight = puzzle->getActualHeight();
    
    std::cout << "Searching in grid of size " << actualWidth << "x" << actualHeight << std::endl;
    
    for (int x = 0; x < actualWidth; x++) {
        for (int y = 0; y < actualHeight; y++) {
            Cell* cell = puzzle->getCell(x, y);
            if (cell && !cell->end.empty()) {
                std::cout << "Found endpoint at " << x << "," << y << " with direction: " << cell->end << std::endl;
                numEndpoints++;
            }
        }
    }
    std::cout << "Found " << numEndpoints << " endpoints" << std::endl;
    return numEndpoints;
}

bool Solver::validatePath(const Path& path) {
    // Create a copy of the puzzle to test the path
    auto testPuzzle = std::make_unique<Puzzle>(*puzzle);
    testPuzzle->clearLines();
    
    // Draw the path
    for (size_t i = 0; i < path.positions.size(); i++) {
        auto [x, y] = path.positions[i];
        if (auto cell = testPuzzle->getCell(x, y)) {
            cell->line = LINE_BLACK;
        }
    }
    
    // Validate the resulting puzzle state
    return testPuzzle->validate();
}