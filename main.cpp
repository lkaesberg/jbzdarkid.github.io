#include "puzzle.hpp"
#include "solver.hpp"
#include <iostream>
#include <fstream>
#include <chrono>

const std::string EXAMPLE_PUZZLE = R"({
    "grid": [
        [{"start": true, "line": 0}, {"line": 0}, {"line": 0}, {"line": 0, "dot": 1}, {"line": 0}],
        [{"line": 0}, {"type": "star", "color": 2}, {"line": 0}, {"type": "star", "color": 2}, {"line": 0}],
        [{"line": 0}, {"line": 0}, {"line": 0}, {"line": 0}, {"line": 0}],
        [{"line": 0}, {"type": "square", "color": 1}, {"line": 0}, {"type": "square", "color": 1}, {"line": 0}],
        [{"line": 0}, {"line": 0}, {"line": 0}, {"line": 0}, {"end": "bottom"}],
        [{"line": 0}, {"type": "triangle", "color": 3, "count": 1}, {"line": 0}, {"type": "star", "color": 3}, {"line": 0}],
        [{"line": 0}, {"line": 0}, {"line": 0}, {"line": 0}, {"line": 0}]
    ],
    "pillar": false
})";

int main() {
    try {
        // Start timing puzzle loading
        auto loadStart = std::chrono::high_resolution_clock::now();
        auto puzzle = Puzzle::deserialize(EXAMPLE_PUZZLE);
        auto loadEnd = std::chrono::high_resolution_clock::now();
        auto loadDuration = std::chrono::duration_cast<std::chrono::microseconds>(loadEnd - loadStart);
        
        std::cout << "Puzzle loaded in " << loadDuration.count() << " microseconds" << std::endl;
        std::cout << "Initial puzzle state:" << std::endl;
        puzzle->printBoard();
        
        // Start timing puzzle solving
        auto solveStart = std::chrono::high_resolution_clock::now();
        Solver solver(std::move(puzzle));
        solver.setMaxSolutions(100000);
        auto solutions = solver.solve();
        auto solveEnd = std::chrono::high_resolution_clock::now();
        auto solveDuration = std::chrono::duration_cast<std::chrono::microseconds>(solveEnd - solveStart);
        
        std::cout << "Puzzle solved in " << solveDuration.count() << " microseconds" << std::endl;
        std::cout << "Found " << solutions.size() << " solutions" << std::endl;
        //return 0;
        for (size_t i = 0; i < solutions.size(); i++) {
            std::cout << "Solution " << (i + 1) << ":" << std::endl;
            
            // Print coordinates
            for (const auto& pos : solutions[i].positions) {
                std::cout << "  (" << pos.first << "," << pos.second << ")";
                if (pos != solutions[i].positions.back()) {
                    std::cout << " ->";
                }
            }
            std::cout << std::endl;
            
            // Draw solution on board
            auto solutionPuzzle = Puzzle::deserialize(EXAMPLE_PUZZLE);
            for (const auto& pos : solutions[i].positions) {
                solutionPuzzle->updateCell(pos.first, pos.second, "line", LINE_BLACK);
            }
            solutionPuzzle->printBoard();
            std::cout << std::endl;
        }
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
} 