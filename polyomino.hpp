#pragma once

#include <vector>
#include <utility>
#include <cstdint>

// Constants for polyomino types
constexpr int POLY_NONE = 0;
constexpr int POLY_NORMAL = 1;
constexpr int POLY_YLOP = 2;

// Rotation bit is at position 20, outside the 4x4 grid (16 bits)
constexpr uint32_t ROTATION_BIT = 1 << 20;

// Functions for bit manipulation of polyshapes
inline uint32_t mask(int x, int y) {
    return 1 << (x * 4 + y);  // Same bit ordering as JS implementation
}

inline bool isSet(uint32_t polyshape, int x, int y) {
    if (x < 0 || y < 0) return false;
    if (x >= 4 || y >= 4) return false;
    return (polyshape & mask(x, y)) != 0;
}

// Check if a polyshape has the rotation bit set
inline bool isRotated(uint32_t polyshape) {
    return (polyshape & ROTATION_BIT) != 0;
}

// Get the size of a polyshape (number of cells)
int getPolySize(uint32_t polyshape);

// Get all rotations of a polyshape
std::vector<uint32_t> getRotations(uint32_t polyshape);

// Rotate a polyshape by a specified number of 90-degree rotations
uint32_t rotatePolyshape(uint32_t polyshape, int count = 1);

// Convert a polyshape to a list of cell coordinates
std::vector<std::pair<int, int>> polyominoFromPolyshape(uint32_t polyshape, bool ylop = false, bool precise = true);

// Convert a list of cell coordinates to a polyshape
uint32_t polyshapeFromPolyomino(const std::vector<std::pair<int, int>>& polyomino);

// Try to place a polyshape on the grid
bool tryPlacePolyshape(const std::vector<std::pair<int, int>>& cells, int x, int y, 
                       std::vector<std::vector<int>>& grid, int sign, 
                       const std::vector<std::pair<int, int>>& region = {});

// Place polyominos in a region
bool placePolys(const std::vector<std::pair<int, int>>& region,
                std::vector<std::vector<int>>& grid,
                const std::vector<std::pair<int, int>>& polyPositions,
                const std::vector<uint32_t>& polyShapes,
                size_t polyIndex = 0); 