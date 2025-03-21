#include "puzzle.hpp"
#include "solver.hpp"
#include "polyomino.hpp"
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
        [{"line": 0}, {"type": "triangle", "color": 3, "count": 1}, {"line": 0}, {"type": "nega"}, {"line": 0}],
        [{"line": 0}, {"line": 0}, {"line": 0}, {"line": 0}, {"line": 0}]
    ],
    "pillar": false
})";

const std::string EXAMPLE_PUZZLE1 = R"({
    "grid": [
        [{"start": true, "line": 0}, {"line": 0}, {"line": 0}, {"line": 0, "dot": 1}, {"line": 0}],
        [{"line": 0}, null,{"line": 0}, null, {"line": 0}],
        [{"line": 0}, {"line": 0}, {"line": 0}, {"line": 0}, {"line": 0}],
        [{"line": 0},null, {"line": 0}, {"type": "nega"}, {"line": 0}],
        [{"line": 0}, {"line": 0}, {"line": 0}, {"line": 0}, {"end": "bottom"}]
    ],
    "pillar": false
})";

const std::string EXAMPLE_PUZZLE2 = R"({
    "grid": [
        [{"start": true, "line": 0}, {"line": 0}, {"line": 0}, {"line": 0, "dot": 1}, {"line": 0}],
        [{"line": 0}, null,{"line": 0, "dot": 1}, null, {"line": 0}],
        [{"line": 0}, {"line": 0}, {"line": 0}, {"line": 0}, {"line": 0}],
        [{"line": 0},null, {"line": 0}, null, {"line": 0}],
        [{"line": 0, "dot": 1}, {"line": 0}, {"line": 0, "dot": 1}, {"line": 0}, {"end": "bottom"}]
    ],
    "pillar": false
})";

const std::string EXAMPLE_PUZZLE3 = R"({
    "grid": [
        [{"start": true, "line": 0}, {"line": 0}, {"line": 0}, {"line": 0, "dot": 1}, {"line": 0}],
        [{"line": 0}, {"type": "nega"},{"line": 0}, null, {"line": 0}],
        [{"line": 0}, {"line": 0}, {"line": 0}, {"line": 0}, {"line": 0}],
        [{"line": 0},{"type": "nega"}, {"line": 0}, {"type": "nega"}, {"line": 0}],
        [{"line": 0}, {"line": 0}, {"line": 0}, {"line": 0}, {"end": "bottom"}]
    ],
    "pillar": false
})";

const std::string EXAMPLE_PUZZLE4 = R"({
    "grid": [
        [{"start": true, "line": 0}, {"line": 0}, {"line": 0}, {"line": 0, "dot": 1}, {"line": 0}],
        [{"line": 0}, {"type": "star", "color": 2}, {"line": 0}, {"type": "star", "color": 2}, {"line": 0}],
        [{"line": 0}, {"line": 0}, {"line": 0}, {"line": 0}, {"line": 0}],
        [{"line": 0}, {"type": "square", "color": 1}, {"line": 0}, {"type": "square", "color": 1}, {"line": 0}],
        [{"line": 0}, {"line": 0}, {"line": 0}, {"line": 0}, {"end": "bottom"}],
        [{"line": 0}, {"type": "triangle", "color": 3, "count": 1}, {"line": 0}, {"type": "poly", "polyshape": 3}, {"line": 0}],
        [{"line": 0}, {"line": 0}, {"line": 0}, {"line": 0}, {"line": 0}]
    ],
    "pillar": false
})";

const std::string EXAMPLE_PUZZLE5 = R"({
    "grid": [
        [{"start": true, "line": 0}, {"line": 0}, {"line": 0}, {"line": 0, "dot": 1}, {"line": 0}],
        [{"line": 0}, {"type": "star", "color": 2}, {"line": 0}, {"type": "star", "color": 2}, {"line": 0}],
        [{"line": 0}, {"line": 0}, {"line": 0}, {"line": 0}, {"line": 0}],
        [{"line": 0}, {"type": "square", "color": 1}, {"line": 0}, {"type": "square", "color": 1}, {"line": 0}],
        [{"line": 0}, {"line": 0}, {"line": 0}, {"line": 0}, {"end": "bottom"}],
        [{"line": 0}, {"type": "nega"}, {"line": 0}, {"type": "poly", "polyshape": 19}, {"line": 0}],
        [{"line": 0}, {"line": 0}, {"line": 0}, {"line": 0}, {"line": 0}]
    ],
    "pillar": false
})";

// Example puzzle with polyominos
const std::string POLY_PUZZLE = R"({
    "grid": [
        [{"start": true, "line": 0}, {"line": 0}, {"line": 0}, {"line": 0}, {"line": 0}],
        [{"line": 0}, {"type": "poly", "polyshape": 3}, {"line": 0}, null, {"line": 0}],
        [{"line": 0}, {"line": 0}, {"line": 0}, {"line": 0}, {"line": 0}],
        [{"line": 0}, {"type": "poly", "polyshape": 3}, {"line": 0}, null, {"line": 0}],
        [{"line": 0}, {"line": 0}, {"line": 0}, {"line": 0}, {"end": "bottom"}]
    ],
    "pillar": false
})";

const std::string POLY_PUZZLE2 = R"({
    "grid": [
        [{"start": true, "line": 0}, {"line": 0}, {"line": 0}, {"line": 0}, {"line": 0}],
        [{"line": 0}, {"type": "poly", "polyshape": 3}, {"line": 0}, null, {"line": 0}],
        [{"line": 0}, {"line": 0}, {"line": 0}, {"line": 0}, {"line": 0}],
        [{"line": 0}, null, {"line": 0}, null, {"line": 0}],
        [{"line": 0}, {"line": 0}, {"line": 0}, {"line": 0}, {"end": "bottom"}]
    ],
    "pillar": false
})";

// Example puzzle with polyominos that should perfectly cancel out
const std::string POLY_CANCEL_PUZZLE = R"({
    "grid": [
        [{"start": true, "line": 0}, {"line": 0}, {"line": 0}, {"line": 0}, {"line": 0}],
        [{"line": 0}, {"type": "poly", "polyshape": 51}, {"line": 0}, null, {"line": 0}],
        [{"line": 0}, {"line": 0}, {"line": 0}, {"line": 0}, {"line": 0}],
        [{"line": 0}, {"type": "ylop", "polyshape": 1}, {"line": 0}, null, {"line": 0}],
        [{"line": 0}, {"line": 0}, {"line": 0}, {"line": 0}, {"end": "bottom"}]
    ],
    "pillar": false
})";

const std::string POLY_CANCEL_PUZZLE2 = R"({
    "grid": [
        [{"start": true, "line": 0}, {"line": 0}, {"line": 0}, {"line": 0}, {"line": 0}],
        [{"line": 0}, {"type": "poly", "polyshape": 19}, {"line": 0}, {"type": "ylop", "polyshape": 1}, {"line": 0}],
        [{"line": 0}, {"line": 0}, {"line": 0}, {"line": 0}, {"line": 0}],
        [{"line": 0}, null, {"line": 0}, null, {"line": 0}],
        [{"line": 0}, {"line": 0}, {"line": 0}, {"line": 0}, {"end": "bottom"}]
    ],
    "pillar": false
})";

// Example with polys and ylops that don't match - this should be invalid
const std::string POLY_INVALID_PUZZLE = R"({
    "grid": [
        [{"start": true, "line": 0}, {"line": 0}, {"line": 0}, {"line": 0}, {"line": 0}],
        [{"line": 0}, {"type": "poly", "polyshape": 3}, {"line": 0}, null, {"line": 0}],
        [{"line": 0}, {"line": 0}, {"line": 0}, {"line": 0}, {"line": 0}],
        [{"line": 0}, {"type": "ylop", "polyshape": 1}, {"line": 0}, null, {"line": 0}],
        [{"line": 0}, {"line": 0}, {"line": 0}, {"line": 0}, {"end": "bottom"}]
    ],
    "pillar": false
})";

int main() {
    try {
        // Demo polyomino puzzle solving
        std::cout << "=== Polyomino Puzzle Solving ===" << std::endl;
        
        // Choose which puzzle to solve
        auto puzzleString = POLY_PUZZLE2;
        
        auto loadStart = std::chrono::high_resolution_clock::now();
        auto puzzle = Puzzle::deserialize(puzzleString);
        auto loadEnd = std::chrono::high_resolution_clock::now();
        auto loadDuration = std::chrono::duration_cast<std::chrono::microseconds>(loadEnd - loadStart);
        
        std::cout << "Puzzle loaded in " << loadDuration.count() << " microseconds" << std::endl;
        std::cout << "Initial puzzle state:" << std::endl;
        puzzle->printBoard();
        
        // Visualize polyominos
        std::cout << "Polyominos in this puzzle:" << std::endl;
        for (int x = 0; x < puzzle->getActualWidth(); x++) {
            for (int y = 0; y < puzzle->getActualHeight(); y++) {
                Cell* cell = puzzle->getCell(x, y);
                if (!cell) continue;
                
                if ((cell->type == "poly" || cell->type == "ylop") && cell->polyshape > 0) {
                    std::cout << "- " << cell->type << " at (" << x << "," << y << ") with shape " 
                             << cell->polyshape << " (size: " << getPolySize(cell->polyshape) << ")" << std::endl;
                    
                    // Print the polyomino shape
                    for (int py = 0; py < 4; py++) {
                        std::cout << "  ";
                        for (int px = 0; px < 4; px++) {
                            std::cout << (isSet(cell->polyshape, px, py) ? "█ " : "· ");
                        }
                        std::cout << std::endl;
                    }
                }
            }
        }
        
        // Start timing puzzle solving
        auto solveStart = std::chrono::high_resolution_clock::now();
        Solver solver(std::move(puzzle));
        solver.setMaxSolutions(100000);
        auto solutions = solver.solve();
        auto solveEnd = std::chrono::high_resolution_clock::now();
        auto solveDuration = std::chrono::duration_cast<std::chrono::microseconds>(solveEnd - solveStart);
        
        std::cout << "Puzzle solved in " << solveDuration.count() << " microseconds" << std::endl;
        std::cout << "Found " << solutions.size() << " solutions" << std::endl;
        
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
            auto solutionPuzzle = Puzzle::deserialize(puzzleString);
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