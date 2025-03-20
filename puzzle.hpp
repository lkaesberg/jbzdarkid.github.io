#pragma once

#include <vector>
#include <string>
#include <memory>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

// Constants for line types
constexpr int LINE_NONE = 0;
constexpr int LINE_BLACK = 1;
constexpr int LINE_BLUE = 2;
constexpr int LINE_YELLOW = 3;

// Constants for gap types
constexpr int GAP_NONE = 0;
constexpr int GAP_BREAK = 1;
constexpr int GAP_FULL = 2;

// Constants for dot types
constexpr int DOT_NONE = 0;
constexpr int DOT_BLACK = 1;
constexpr int DOT_BLUE = 2;
constexpr int DOT_YELLOW = 3;

// Constants for path directions
constexpr int PATH_NONE = 0;
constexpr int PATH_LEFT = 1;
constexpr int PATH_RIGHT = 2;
constexpr int PATH_TOP = 3;
constexpr int PATH_BOTTOM = 4;

// Constants for negation types
constexpr int NEGA_NONE = 0;
constexpr int NEGA_BLACK = 1;
constexpr int NEGA_WHITE = 2;

// Forward declarations
class Cell;
class Puzzle;

// Represents a single cell in the puzzle grid
class Cell {
public:
    int line = LINE_NONE;
    int gap = GAP_NONE;
    int dot = DOT_NONE;
    bool start = false;
    std::string end;
    std::string type;
    int color = 0;
    int count = 0;
    int polyshape = 0;
    std::string dir;
    int nega = NEGA_NONE;  // Added negation type
};

// Main puzzle class that represents the entire puzzle grid
class Puzzle {
public:
    Puzzle(int width, int height, bool pillar = false);
    static std::unique_ptr<Puzzle> deserialize(const std::string& jsonStr);
    std::string serialize() const;
    
    // Core puzzle functionality
    Cell* getCell(int x, int y);
    void updateCell(int x, int y, const std::string& key, const json& value);
    void clearLines();
    std::vector<std::vector<std::pair<int, int>>> getRegions();
    std::vector<std::pair<int, int>> getRegion(int x, int y);
    
    // Validation
    bool validate();
    
    // Getters
    int getWidth() const { return width; }
    int getHeight() const { return height; }
    bool isPillar() const { return pillar; }
    int getActualWidth() const { return grid.size(); }
    int getActualHeight() const { return grid[0].size(); }
    
    // Grid wrapping
    int _mod(int val) const;
    
    // Additional functionality
    void printBoard() const;
    
private:
    std::vector<std::vector<Cell>> grid;
    int width;
    int height;
    bool pillar;
    
    // Helper methods
    bool _safeCell(int x, int y) const;
    void _floodFill(int x, int y, std::vector<std::pair<int, int>>& region);
    void _floodFillOutside(int x, int y);
}; 