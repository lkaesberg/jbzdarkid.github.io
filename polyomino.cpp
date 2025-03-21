#include "polyomino.hpp"
#include <algorithm>
#include <limits>

// Count the number of cells in a polyshape
int getPolySize(uint16_t polyshape) {
    int size = 0;
    for (int x = 0; x < 4; x++) {
        for (int y = 0; y < 4; y++) {
            if (isSet(polyshape, x, y)) size++;
        }
    }
    return size;
}

// Get all rotations of a polyshape
std::vector<uint16_t> getRotations(uint16_t polyshape) {
    if (!isRotated(polyshape)) {
        return {polyshape}; // If not marked as rotatable, return only the original shape
    }

    std::vector<uint16_t> rotations(4, 0);
    
    // Generate all 4 possible 90-degree rotations
    for (int x = 0; x < 4; x++) {
        for (int y = 0; y < 4; y++) {
            if (isSet(polyshape, x, y)) {
                rotations[0] ^= mask(x, y);           // Original
                rotations[1] ^= mask(y, 3-x);         // 90° clockwise
                rotations[2] ^= mask(3-x, 3-y);       // 180°
                rotations[3] ^= mask(3-y, x);         // 270° clockwise
            }
        }
    }

    return rotations;
}

// Rotate a polyshape by a specified number of 90-degree rotations
uint16_t rotatePolyshape(uint16_t polyshape, int count) {
    auto rotations = getRotations(polyshape | ROTATION_BIT);
    return rotations[count % 4];
}

// Convert a polyshape to a list of cell coordinates
std::vector<std::pair<int, int>> polyominoFromPolyshape(uint16_t polyshape, bool ylop, bool precise) {
    std::vector<std::pair<int, int>> polyomino;
    
    // Find the top-left cell
    std::pair<int, int> topLeft = {-1, -1};
    for (int y = 0; y < 4; y++) {
        for (int x = 0; x < 4; x++) {
            if (isSet(polyshape, x, y)) {
                topLeft = {x, y};
                break;
            }
        }
        if (topLeft.first != -1) break;
    }
    
    if (topLeft.first == -1) return {}; // Empty polyomino
    
    for (int x = 0; x < 4; x++) {
        for (int y = 0; y < 4; y++) {
            if (!isSet(polyshape, x, y)) continue;
            
            // Add the cell, transformed to the coordinate system with topLeft at origin
            polyomino.push_back({2*(x - topLeft.first), 2*(y - topLeft.second)});
            
            if (precise) {
                if (ylop) {
                    // Ylops fill up/left if no adjacent cell, and always fill bottom/right
                    if (!isSet(polyshape, x - 1, y)) {
                        polyomino.push_back({2*(x - topLeft.first) - 1, 2*(y - topLeft.second)});
                    }
                    if (!isSet(polyshape, x, y - 1)) {
                        polyomino.push_back({2*(x - topLeft.first), 2*(y - topLeft.second) - 1});
                    }
                    polyomino.push_back({2*(x - topLeft.first) + 1, 2*(y - topLeft.second)});
                    polyomino.push_back({2*(x - topLeft.first), 2*(y - topLeft.second) + 1});
                } else {
                    // Normal polys only fill bottom/right if there is an adjacent cell
                    if (isSet(polyshape, x + 1, y)) {
                        polyomino.push_back({2*(x - topLeft.first) + 1, 2*(y - topLeft.second)});
                    }
                    if (isSet(polyshape, x, y + 1)) {
                        polyomino.push_back({2*(x - topLeft.first), 2*(y - topLeft.second) + 1});
                    }
                }
            }
        }
    }
    
    return polyomino;
}

// Convert a list of cell coordinates to a polyshape
uint16_t polyshapeFromPolyomino(const std::vector<std::pair<int, int>>& polyomino) {
    // Find the top-left cell
    int minX = std::numeric_limits<int>::max();
    int minY = std::numeric_limits<int>::max();
    
    for (const auto& pos : polyomino) {
        // We only care about cells (positions with both coordinates odd), not edges or intersections
        if (pos.first % 2 != 1 || pos.second % 2 != 1) continue;
        
        minX = std::min(minX, pos.first);
        minY = std::min(minY, pos.second);
    }
    
    if (minX == std::numeric_limits<int>::max()) return 0; // Empty polyomino
    
    uint16_t polyshape = 0;
    for (const auto& pos : polyomino) {
        if (pos.first % 2 != 1 || pos.second % 2 != 1) continue;
        
        // Convert from puzzle coordinates to polyshape coordinates
        int x = (pos.first - minX) / 2;
        int y = (pos.second - minY) / 2;
        
        polyshape |= mask(x, y);
    }
    
    return polyshape;
}

// Try to place a polyshape on the grid
bool tryPlacePolyshape(const std::vector<std::pair<int, int>>& cells, int x, int y, 
                      std::vector<std::vector<int>>& grid, int sign, 
                      const std::vector<std::pair<int, int>>& region) {
    std::vector<std::pair<int, int>> cellsToUpdate;
    std::vector<int> originalValues;
    
    // First check if placement is valid
    for (const auto& cell : cells) {
        int newX = x + cell.first;
        int newY = y + cell.second;
        
        // Check grid bounds
        if (newX < 0 || newY < 0 || newX >= static_cast<int>(grid.size()) || 
            newY >= static_cast<int>(grid[0].size())) {
            return false;
        }
        
        // Check if cell is in region (if region is specified)
        if (!region.empty()) {
            bool inRegion = false;
            for (const auto& pos : region) {
                if (pos.first == newX && pos.second == newY) {
                    inRegion = true;
                    break;
                }
            }
            if (!inRegion) {
                return false;
            }
        }
        
        // Content cells (odd coordinates) should be valid for placement
        if (newX % 2 == 1 && newY % 2 == 1) {
            // Store cell for update
            cellsToUpdate.push_back({newX, newY});
            originalValues.push_back(grid[newX][newY]);
        }
    }
    
    // Apply the update
    for (size_t i = 0; i < cellsToUpdate.size(); i++) {
        const auto& cell = cellsToUpdate[i];
        grid[cell.first][cell.second] += sign;
    }
    
    return true;
}

// Place polyominos in a region
bool placePolys(const std::vector<std::pair<int, int>>& region,
                std::vector<std::vector<int>>& grid,
                const std::vector<std::pair<int, int>>& polyPositions,
                const std::vector<uint16_t>& polyShapes,
                size_t polyIndex) {
    // Base case: all polyominos placed
    if (polyIndex >= polyShapes.size()) {
        // Check if all cells that need coverage are covered
        for (const auto& pos : region) {
            if (pos.first % 2 == 1 && pos.second % 2 == 1) { // Only check cell positions
                if (grid[pos.first][pos.second] < 0) {
                    return false; // Uncovered cell
                }
            }
        }
        return true; // All cells covered
    }
    
    // Get the next polyomino to place
    uint16_t polyShape = polyShapes[polyIndex];
    
    // Try each rotation
    auto rotations = getRotations(polyShape | ROTATION_BIT);
    for (auto rotation : rotations) {
        auto cells = polyominoFromPolyshape(rotation);
        
        // Try to place at the designated position
        int baseX = polyPositions[polyIndex].first;
        int baseY = polyPositions[polyIndex].second;
        
        if (tryPlacePolyshape(cells, baseX, baseY, grid, 1, region)) {
            // Recursively try to place the remaining polyominos
            if (placePolys(region, grid, polyPositions, polyShapes, polyIndex + 1)) {
                return true;
            }
            
            // If we couldn't place all, undo this placement and try next rotation
            tryPlacePolyshape(cells, baseX, baseY, grid, -1, region);
        }
    }
    
    // No valid placement found
    return false;
} 