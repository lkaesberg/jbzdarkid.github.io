#include "puzzle.hpp"
#include "polyomino.hpp"
#include <stdexcept>
#include <nlohmann/json.hpp>
#include <iostream>

using json = nlohmann::json;

Puzzle::Puzzle(int w, int h, bool p) : width(w), height(h), pillar(p) {    
    // The actual grid size is 2*w+1 x 2*h+1
    int actualWidth = 2 * w + 1;
    int actualHeight = 2 * h + 1;
    
    grid.resize(actualWidth);
    for (int x = 0; x < actualWidth; x++) {
        grid[x].resize(actualHeight);
        for (int y = 0; y < actualHeight; y++) {
            if (x % 2 == 1 && y % 2 == 1) {
                grid[x][y] = Cell(); // Empty cell
            } else {
                Cell cell;
                cell.type = "line";
                grid[x][y] = cell;
            }
        }
    }
}

std::unique_ptr<Puzzle> Puzzle::deserialize(const std::string& jsonStr) {    
    try {
        json j = json::parse(jsonStr);
        
        // Calculate grid dimensions
        if (!j.contains("grid") || !j["grid"].is_array() || j["grid"].empty() || !j["grid"][0].is_array()) {
            throw std::runtime_error("Invalid grid format in JSON");
        }
        
        // The actual grid size is 2*w+1 x 2*h+1
        int actualWidth = j["grid"].size();
        int actualHeight = j["grid"][0].size();
        bool isPillar = j.value("pillar", false);
        std::cout << "Actual grid size: " << actualWidth << "x" << actualHeight << " (pillar: " << isPillar << ")" << std::endl;
        
        // Calculate the logical dimensions (for the puzzle cells)
        int w = (actualWidth - 1) / 2;
        int h = (actualHeight - 1) / 2;
        
        // Validate grid dimensions
        if (w <= 0 || h <= 0) {
            throw std::runtime_error("Invalid grid dimensions");
        }
        
        // Validate that all rows have the same length
        for (size_t i = 0; i < j["grid"].size(); i++) {
            if (j["grid"][i].size() != actualHeight) {
                throw std::runtime_error("Inconsistent row lengths in grid");
            }
        }
        
        auto puzzle = std::make_unique<Puzzle>(w, h, isPillar);        
        // Copy grid data
        for (int x = 0; x < actualWidth; x++) {            
            // Validate row bounds
            if (x >= puzzle->grid.size()) {
                std::cerr << "Row index " << x << " out of bounds (grid size: " << puzzle->grid.size() << ")" << std::endl;
                throw std::runtime_error("Row index out of bounds");
            }
            
            for (int y = 0; y < actualHeight; y++) {
                // Validate column bounds
                if (y >= puzzle->grid[x].size()) {
                    std::cerr << "Column index " << y << " out of bounds for row " << x 
                             << " (row size: " << puzzle->grid[x].size() << ")" << std::endl;
                    throw std::runtime_error("Column index out of bounds");
                }
                
                auto& cell = j["grid"][x][y];
                if (cell.is_null()) {
                    continue;
                }
                
                try {
                    Cell& targetCell = puzzle->grid[x][y];
                    
                    if (cell.contains("start")) {
                        targetCell.start = cell["start"];
                    }
                    if (cell.contains("end")) {
                        targetCell.end = cell["end"];
                    }
                    if (cell.contains("type")) {
                        targetCell.type = cell["type"];
                    }
                    if (cell.contains("color")) {
                        targetCell.color = cell["color"];
                    }
                    if (cell.contains("count")) {
                        targetCell.count = cell["count"];
                    }
                    if (cell.contains("polyshape")) {
                        targetCell.polyshape = cell["polyshape"];
                    }
                    if (cell.contains("line")) {
                        targetCell.line = cell["line"];
                    }
                    if (cell.contains("gap")) {
                        targetCell.gap = cell["gap"];
                    }
                    if (cell.contains("dot")) {
                        targetCell.dot = cell["dot"];
                    }
                    // Handle negation symbols
                    if (cell.contains("type") && cell["type"] == "nega") {
                        if (cell.contains("color")) {
                            std::string color = cell["color"];
                            if (color == "white") {
                                targetCell.nega = NEGA_WHITE;
                            } else if (color == "black") {
                                targetCell.nega = NEGA_BLACK;
                            } else {
                                targetCell.nega = NEGA_BLACK; // Default to black if invalid color
                            }
                        } else {
                            targetCell.nega = NEGA_BLACK; // Default to black if no color specified
                        }
                    }
                } catch (const std::exception& e) {
                    std::cerr << "Error processing cell at " << x << "," << y << ": " << e.what() << std::endl;
                    throw;
                }
            }
        }
        
        return puzzle;
        
    } catch (const json::parse_error& e) {
        std::cerr << "JSON parse error: " << e.what() << std::endl;
        throw;
    } catch (const std::exception& e) {
        std::cerr << "Error during deserialization: " << e.what() << std::endl;
        throw;
    }
}

std::string Puzzle::serialize() const {
    json j;
    j["width"] = width;
    j["height"] = height;
    j["pillar"] = pillar;
    
    json gridJson;
    for (const auto& row : grid) {
        json rowJson;
        for (const auto& cell : row) {
            json cellJson;
            if (cell.start) cellJson["start"] = true;
            if (!cell.end.empty()) cellJson["end"] = cell.end;
            if (!cell.type.empty()) cellJson["type"] = cell.type;
            if (cell.color != 0) cellJson["color"] = cell.color;
            if (cell.count != 0) cellJson["count"] = cell.count;
            if (cell.polyshape != 0) cellJson["polyshape"] = cell.polyshape;
            if (cell.line != LINE_NONE) cellJson["line"] = cell.line;
            if (cell.gap != GAP_NONE) cellJson["gap"] = cell.gap;
            if (cell.dot != DOT_NONE) cellJson["dot"] = cell.dot;
            // Add negation information
            if (cell.nega != NEGA_NONE) {
                cellJson["color"] = (cell.nega == NEGA_WHITE) ? "white" : "black";
            }
            rowJson.push_back(cellJson);
        }
        gridJson.push_back(rowJson);
    }
    j["grid"] = gridJson;
    
    return j.dump();
}

Cell* Puzzle::getCell(int x, int y) {
    x = _mod(x);
    if (!_safeCell(x, y)) {
        std::cout << "Cell access out of bounds: " << x << "," << y << std::endl;
        return nullptr;
    }
    return &grid[x][y];
}

void Puzzle::updateCell(int x, int y, const std::string& key, const json& value) {
    x = _mod(x);
    if (!_safeCell(x, y)) return;
    
    Cell& cell = grid[x][y];
    if (key == "line") cell.line = value;
    else if (key == "gap") cell.gap = value;
    else if (key == "dot") cell.dot = value;
    else if (key == "color") cell.color = value;
    else if (key == "start") cell.start = value;
    else if (key == "end") cell.end = value;
    else if (key == "type") cell.type = value;
    else if (key == "count") cell.count = value;
    else if (key == "polyshape") cell.polyshape = value;
    else if (key == "dir") cell.dir = value;
}

void Puzzle::clearLines() {
    for (int x = 0; x < width; x++) {
        for (int y = 0; y < height; y++) {
            updateCell(x, y, "line", LINE_NONE);
            grid[x][y].dir.clear();
        }
    }
}

int Puzzle::_mod(int val) const {
    if (!pillar) return val;
    return (val + width * height * 2) % width;
}

bool Puzzle::_safeCell(int x, int y) const {
    if (x < 0 || x >= grid.size()) return false;
    if (y < 0 || y >= grid[0].size()) return false;
    return true;
}

void Puzzle::_floodFill(int x, int y, std::vector<std::pair<int, int>>& region) {
    Cell* cell = getCell(x, y);
    if (!cell) return;
    
    // Skip if cell is already in the region
    for (const auto& [rx, ry] : region) {
        if (rx == x && ry == y) return;
    }
    
    // For line cells, we can only pass through if there's NO line
    if ((x % 2 == 0 || y % 2 == 0) && cell->line != LINE_NONE) {
        return;
    }
    
    // Add cell to region
    region.push_back({x, y});
    
    // Check all adjacent cells
    if (y < grid[0].size() - 1) _floodFill(x, y + 1, region);
    if (y > 0) _floodFill(x, y - 1, region);
    if (x < grid.size() - 1) _floodFill(x + 1, y, region);
    else if (pillar) _floodFill(0, y, region);
    if (x > 0) _floodFill(x - 1, y, region);
    else if (pillar) _floodFill(grid.size() - 1, y, region);
}

void Puzzle::_floodFillOutside(int x, int y) {
    Cell* cell = getCell(x, y);
    if (!cell || cell->line == LINE_NONE) return;
    
    if (x % 2 != y % 2 && cell->gap != GAP_FULL) return;
    if (x % 2 == 0 && y % 2 == 0 && cell->dot != DOT_NONE) return;
    
    cell->line = LINE_NONE;
    
    if (x % 2 == 0 && y % 2 == 0) return;
    
    if (y < height - 1) _floodFillOutside(x, y + 1);
    if (y > 0) _floodFillOutside(x, y - 1);
    if (x < width - 1) _floodFillOutside(x + 1, y);
    else if (pillar) _floodFillOutside(0, y);
    if (x > 0) _floodFillOutside(x - 1, y);
    else if (pillar) _floodFillOutside(width - 1, y);
}

std::vector<std::vector<std::pair<int, int>>> Puzzle::getRegions() {
    std::vector<std::vector<std::pair<int, int>>> regions;
    
    // Create a copy of the grid for flood fill
    auto tempGrid = grid;
    
    // Find regions starting from content cells (squares, etc.)
    for (int x = 1; x < grid.size(); x += 2) {
        for (int y = 1; y < grid[0].size(); y += 2) {
            bool alreadyInRegion = false;
            
            // Check if this cell is already in a region
            for (const auto& region : regions) {
                for (const auto& [rx, ry] : region) {
                    if (rx == x && ry == y) {
                        alreadyInRegion = true;
                        break;
                    }
                }
                if (alreadyInRegion) break;
            }
            
            if (alreadyInRegion) continue;
            
            // Start a new region from this content cell
            std::vector<std::pair<int, int>> region;
            _floodFill(x, y, region);
            
            if (!region.empty()) {
                regions.push_back(region);
            }
        }
    }
    return regions;
}

std::vector<std::pair<int, int>> Puzzle::getRegion(int x, int y) {
    x = _mod(x);
    if (!_safeCell(x, y)) return {};
    
    // Create a copy of the grid for flood fill
    auto tempGrid = grid;
    
    std::vector<std::pair<int, int>> region;
    _floodFill(x, y, region);
    
    // Restore the grid
    grid = tempGrid;
    
    return region;
}

bool Puzzle::validate() {
    // First check for gaps in the path
    for (int x = 0; x < grid.size(); x++) {
        for (int y = 0; y < grid[0].size(); y++) {
            Cell* cell = getCell(x, y);
            if (!cell) continue;
            
            // Skip content cells and cells with lines
            if (x % 2 == 1 && y % 2 == 1) continue;
            if (cell->line != LINE_NONE) continue;
            
            // Count adjacent lines and check their directions
            int adjacentLines = 0;
            bool hasVertical = false;
            bool hasHorizontal = false;
            
            // Check vertical neighbors
            if (y > 0 && getCell(x, y - 1) && getCell(x, y - 1)->line != LINE_NONE) {
                adjacentLines++;
                hasVertical = true;
            }
            if (y < grid[0].size() - 1 && getCell(x, y + 1) && getCell(x, y + 1)->line != LINE_NONE) {
                adjacentLines++;
                hasVertical = true;
            }
            
            // Check horizontal neighbors
            if (x > 0 && getCell(x - 1, y) && getCell(x - 1, y)->line != LINE_NONE) {
                adjacentLines++;
                hasHorizontal = true;
            }
            if (x < grid.size() - 1 && getCell(x + 1, y) && getCell(x + 1, y)->line != LINE_NONE) {
                adjacentLines++;
                hasHorizontal = true;
            }
            
            // It's only a gap if we have lines in both directions
            if (adjacentLines >= 2 && hasVertical && hasHorizontal) {
                std::cout << "Found gap in path at " << x << "," << y << std::endl;
                return false;
            }
        }
    }

    
    // Get all regions
    auto regions = getRegions();
    
    // Check each region
    for (const auto& region : regions) {
        std::vector<std::pair<int, int>> squares;
        std::vector<std::pair<int, int>> stars;
        std::vector<std::pair<int, int>> triangles;
        std::vector<std::pair<int, int>> negations;
        std::vector<std::pair<int, int>> polys;     // Regular polyominos
        std::vector<std::pair<int, int>> ylops;     // Inverse polyominos
        std::map<int, int> coloredObjects;  // color -> count
        int squareColor = -1;  // -1 means no squares found yet
        std::vector<std::pair<int, int>> regionInvalidElements;
        
        // First pass: collect all symbols and check for uncovered dots
        for (const auto& [x, y] : region) {
            Cell* cell = getCell(x, y);
            if (!cell) continue;
            
            // Check for uncovered dots in this region
            if (cell->dot) {
                if (cell->line == LINE_NONE) {
                    regionInvalidElements.push_back({x, y});
                }
            }
            
            // Only check colored objects at odd coordinates
            if (x % 2 == 1 && y % 2 == 1) {
                if (cell->type == "square") {
                    if (squareColor == -1) {
                        squareColor = cell->color;
                    }
                    squares.push_back({x, y});
                    coloredObjects[cell->color]++;
                }
                else if (cell->type == "star") {
                    stars.push_back({x, y});
                    coloredObjects[cell->color]++;
                }
                else if (cell->type == "triangle") {
                    triangles.push_back({x, y});
                }
                else if (cell->type == "nega") {
                    negations.push_back({x, y});
                }
                else if (cell->type == "poly") {
                    polys.push_back({x, y});
                }
                else if (cell->type == "ylop") {
                    ylops.push_back({x, y});
                }
            }
        }

        // Second pass: check for invalid elements
        // Check squares of different colors
        for (const auto& [x, y] : squares) {
            Cell* cell = getCell(x, y);
            if (cell && cell->color != squareColor) {
                regionInvalidElements.push_back({x, y});
            }
        }

        // Check stars (must come in pairs)
        for (const auto& [color, count] : coloredObjects) {
            if (count == 1 || count > 2) {
                // Add all stars of this color to invalid elements
                for (const auto& [x, y] : stars) {
                    Cell* cell = getCell(x, y);
                    if (cell && cell->color == color) {
                        regionInvalidElements.push_back({x, y});
                    }
                }
            }
        }

        // Check triangles
        for (const auto& [x, y] : triangles) {
            Cell* cell = getCell(x, y);
            if (!cell) continue;
            
            // Count adjacent lines
            int adjacentLines = 0;
            if (getCell(x - 1, y) && getCell(x - 1, y)->line != LINE_NONE) adjacentLines++;
            if (getCell(x + 1, y) && getCell(x + 1, y)->line != LINE_NONE) adjacentLines++;
            if (getCell(x, y - 1) && getCell(x, y - 1)->line != LINE_NONE) adjacentLines++;
            if (getCell(x, y + 1) && getCell(x, y + 1)->line != LINE_NONE) adjacentLines++;
            
            if (adjacentLines != cell->count) {
                regionInvalidElements.push_back({x, y});
            }
        }

        // Check polyominos and ylops
        if (!polys.empty() || !ylops.empty()) {
            // Count region size (only odd-coordinate cells)
            int regionSize = 0;
            for (const auto& pos : region) {
                if (pos.first % 2 == 1 && pos.second % 2 == 1) {
                    regionSize++;
                }
            }
            
            // Calculate total poly and ylop sizes
            int polySize = 0;  // Total size of all polys
            int ylopSize = 0;  // Total size of all ylops
            std::vector<uint16_t> polyShapes;
            std::vector<uint16_t> ylopShapes;
            std::vector<std::pair<int, int>> polyPositions;
            std::vector<std::pair<int, int>> ylopPositions;
            
            for (const auto& [x, y] : polys) {
                Cell* cell = getCell(x, y);
                if (cell && cell->polyshape > 0) {
                    polyShapes.push_back(cell->polyshape);
                    polyPositions.push_back({x, y});
                    polySize += getPolySize(cell->polyshape);
                }
            }
            
            for (const auto& [x, y] : ylops) {
                Cell* cell = getCell(x, y);
                if (cell && cell->polyshape > 0) {
                    ylopShapes.push_back(cell->polyshape);
                    ylopPositions.push_back({x, y});
                    ylopSize += getPolySize(cell->polyshape);
                }
            }
            
            // If we have polyominos or ylops, make sure they correctly fit the region
            if (!polyShapes.empty() || !ylopShapes.empty()) {
                std::cout << "Region size: " << regionSize << ", Poly size: " << polySize << ", Ylop size: " << ylopSize << std::endl;
                
                // Check if the math works out: poly_size = region_size + ylop_size
                // (Polys must cover the original region plus the ylop extension)
                if (polySize != regionSize + ylopSize) {
                    std::cout << "Poly size (" << polySize << ") doesn't match region size (" << regionSize 
                              << ") + ylop size (" << ylopSize << ") = " << (regionSize + ylopSize) << std::endl;
                    return false;
                }
                
                // Create working grid for validation
                std::vector<std::vector<int>> workingGrid(grid.size(), std::vector<int>(grid[0].size(), 0));
                
                // Mark cells in the region as needing coverage (-1)
                for (const auto& pos : region) {
                    if (pos.first % 2 == 1 && pos.second % 2 == 1) {
                        workingGrid[pos.first][pos.second] = -1; // Region cells start as -1
                    }
                }
                
                // Place ylops to extend the region
                for (size_t i = 0; i < ylopShapes.size(); i++) {
                    auto shape = ylopShapes[i];
                    
                    std::cout << "Placing ylop shape " << shape << std::endl;
                    
                    // Find all positions adjacent to the region to try placing the ylop
                    std::vector<std::pair<int, int>> candidatePositions;
                    
                    // First collect all cells adjacent to the region
                    for (const auto& pos : region) {
                        if (pos.first % 2 == 1 && pos.second % 2 == 1) {
                            // Check all 4 adjacent cells (if they're not in the region)
                            std::vector<std::pair<int, int>> adjacentPositions = {
                                {pos.first + 2, pos.second},
                                {pos.first - 2, pos.second},
                                {pos.first, pos.second + 2},
                                {pos.first, pos.second - 2}
                            };
                            
                            for (const auto& adjPos : adjacentPositions) {
                                // Skip if outside grid
                                if (adjPos.first < 0 || adjPos.second < 0 || 
                                    adjPos.first >= static_cast<int>(grid.size()) || 
                                    adjPos.second >= static_cast<int>(grid[0].size())) {
                                    continue;
                                }
                                
                                // Skip if part of the region
                                bool inRegion = false;
                                for (const auto& regionPos : region) {
                                    if (regionPos.first == adjPos.first && regionPos.second == adjPos.second) {
                                        inRegion = true;
                                        break;
                                    }
                                }
                                if (inRegion) continue;
                                
                                // Add to candidate positions
                                bool alreadyAdded = false;
                                for (const auto& candPos : candidatePositions) {
                                    if (candPos.first == adjPos.first && candPos.second == adjPos.second) {
                                        alreadyAdded = true;
                                        break;
                                    }
                                }
                                if (!alreadyAdded) {
                                    candidatePositions.push_back(adjPos);
                                }
                            }
                        }
                    }
                    
                    // If no adjacent positions, also try the original ylop position
                    if (candidatePositions.empty() && i < ylopPositions.size()) {
                        candidatePositions.push_back(ylopPositions[i]);
                    }
                    
                    std::cout << "Found " << candidatePositions.size() << " candidate positions for ylop" << std::endl;
                    
                    // Try to place the ylop at any valid position
                    bool placed = false;
                    std::vector<uint16_t> rotations = getRotations(shape | ROTATION_BIT);
                    
                    for (const auto& position : candidatePositions) {
                        for (auto rotation : rotations) {
                            auto cells = polyominoFromPolyshape(rotation, true); // ylop=true
                            std::vector<std::pair<int, int>> cellsToConvert;
                            
                            // Only mark cells outside the region
                            bool valid = true;
                            for (const auto& cell : cells) {
                                int newX = position.first + cell.first;
                                int newY = position.second + cell.second;
                                
                                // Skip if outside grid
                                if (newX < 0 || newY < 0 || newX >= static_cast<int>(grid.size()) || 
                                    newY >= static_cast<int>(grid[0].size())) {
                                    continue;
                                }
                                
                                // Only consider actual cells (odd coordinates)
                                if (newX % 2 != 1 || newY % 2 != 1) {
                                    continue;
                                }
                                
                                // Check if in region
                                bool inRegion = false;
                                for (const auto& pos : region) {
                                    if (pos.first == newX && pos.second == newY) {
                                        inRegion = true;
                                        break;
                                    }
                                }
                                
                                // If the cell is already in the region, this isn't valid
                                if (inRegion) {
                                    valid = false;
                                    break;
                                }
                                
                                // This is a cell we should convert
                                cellsToConvert.push_back({newX, newY});
                            }
                            
                            if (valid && !cellsToConvert.empty()) {
                                // Mark cells outside the region as needing coverage (-1)
                                for (const auto& cell : cellsToConvert) {
                                    std::cout << "  Marking cell " << cell.first << "," << cell.second 
                                            << " as needing coverage (ylop extension)" << std::endl;
                                    workingGrid[cell.first][cell.second] = -1;
                                }
                                placed = true;
                                break;
                            }
                        }
                        if (placed) break;
                    }
                    
                    if (!placed) {
                        std::cout << "Failed to place ylop shape " << shape << " anywhere" << std::endl;
                        return false;
                    }
                }
                
                // Place regular polyominos to provide needed coverage
                for (size_t i = 0; i < polyShapes.size(); i++) {
                    auto shape = polyShapes[i];
                    
                    std::cout << "Placing poly shape " << shape << std::endl;
                    
                    // Collect all cells in the region that need coverage
                    std::vector<std::pair<int, int>> candidatePositions;
                    for (const auto& pos : region) {
                        if (pos.first % 2 == 1 && pos.second % 2 == 1 && workingGrid[pos.first][pos.second] == -1) {
                            candidatePositions.push_back(pos);
                        }
                    }
                    
                    // If no positions in region, also include the extended region from ylops
                    if (candidatePositions.empty()) {
                        for (int x = 1; x < grid.size(); x += 2) {
                            for (int y = 1; y < grid[0].size(); y += 2) {
                                if (workingGrid[x][y] == -1) {
                                    candidatePositions.push_back({x, y});
                                }
                            }
                        }
                    }
                    
                    // If still no positions, also try the original poly position
                    if (candidatePositions.empty() && i < polyPositions.size()) {
                        candidatePositions.push_back(polyPositions[i]);
                    }
                    
                    std::cout << "Found " << candidatePositions.size() << " candidate positions for poly" << std::endl;
                    
                    // Try to place the poly at any valid position
                    bool placed = false;
                    std::vector<uint16_t> rotations = getRotations(shape | ROTATION_BIT);
                    
                    for (const auto& position : candidatePositions) {
                        for (auto rotation : rotations) {
                            auto cells = polyominoFromPolyshape(rotation, false);
                            std::vector<std::pair<int, int>> cellsToUpdate;
                            std::vector<int> originalValues;
                            
                            // Check if this placement is valid
                            bool valid = true;
                            for (const auto& cell : cells) {
                                int newX = position.first + cell.first;
                                int newY = position.second + cell.second;
                                
                                // Skip if outside grid
                                if (newX < 0 || newY < 0 || newX >= static_cast<int>(grid.size()) || 
                                    newY >= static_cast<int>(grid[0].size())) {
                                    valid = false;
                                    break;
                                }
                                
                                // Only consider actual cells (odd coordinates)
                                if (newX % 2 != 1 || newY % 2 != 1) {
                                    continue;
                                }
                                
                                // Poly can only cover cells that need coverage (-1)
                                if (workingGrid[newX][newY] != -1) {
                                    valid = false;
                                    break;
                                }
                                
                                // This is a cell we can update
                                cellsToUpdate.push_back({newX, newY});
                                originalValues.push_back(workingGrid[newX][newY]);
                            }
                            
                            if (valid && !cellsToUpdate.empty()) {
                                // Mark cells as covered (0)
                                for (const auto& cell : cellsToUpdate) {
                                    std::cout << "  Covering cell " << cell.first << "," << cell.second << std::endl;
                                    workingGrid[cell.first][cell.second] = 0;
                                }
                                placed = true;
                                break;
                            }
                        }
                        if (placed) break;
                    }
                    
                    if (!placed) {
                        std::cout << "Failed to place poly shape " << shape << " anywhere" << std::endl;
                        return false;
                    }
                }
                
                // Check if all cells (original region + ylop extensions) have been correctly covered
                for (int x = 1; x < grid.size(); x += 2) {
                    for (int y = 1; y < grid[0].size(); y += 2) {
                        if (workingGrid[x][y] < 0) {
                            std::cout << "Cell at " << x << "," << y 
                                     << " not covered (value: " << workingGrid[x][y] << ")" << std::endl;
                            return false;
                        }
                    }
                }
            }
        }

        // If there are no negations in this region, check if there are any invalid elements
        if (negations.empty()) {
            if (!regionInvalidElements.empty()) {
                return false;
            }
            continue;
        }

        // Handle negations
        // First, pair up negations that can cancel each other
        int remainingNegations = negations.size();
        if (remainingNegations >= 2) {
            // Each pair of negations can cancel each other
            remainingNegations = remainingNegations % 2;
        }

        // Any remaining negations must each cancel exactly one invalid element
        if (remainingNegations > 0) {
            // If there are no invalid elements but we have remaining negations, the puzzle is invalid
            if (regionInvalidElements.empty()) {
                return false;
            }

            // Each remaining negation must cancel exactly one invalid element
            if (remainingNegations != regionInvalidElements.size()) {
                return false;
            }
        } else {
            // If all negations cancelled each other, there should be no invalid elements
            if (!regionInvalidElements.empty()) {
                return false;
            }
        }
    }
    
    return true;
}

void Puzzle::printBoard() const {
    // Print column numbers
    std::cout << "   ";
    for (int x = 0; x < grid.size(); x++) {
        std::cout << x % 10 << " ";
    }
    std::cout << "\n";
    
    // Print the grid
    for (int y = 0; y < grid[0].size(); y++) {
        // Print row number
        std::cout << y % 10 << "  ";
        
        for (int x = 0; x < grid.size(); x++) {
            const auto& cell = grid[x][y];
            
            if (cell.start) {
                std::cout << "S ";
            } else if (!cell.end.empty()) {
                std::cout << "E ";
            } else if (cell.dot > DOT_NONE) {
                std::cout << "• ";
            } else if (cell.line > LINE_NONE) {
                std::cout << "█ ";
            } else if (cell.gap > GAP_NONE) {
                std::cout << "╌ ";
            } else if (cell.type == "line") {
                std::cout << "· ";
            } else if (cell.type == "square") {
                // Print squares with their color number
                std::cout << "s" << cell.color;
            } else if (cell.type == "star") {
                // Print stars with their color number
                std::cout << "*" << cell.color;
            } else if (cell.type == "triangle") {
                // Print triangles with their count
                std::cout << "△" << cell.count;
            } else if (cell.type == "nega") {
                // Print negation symbols (N for black, n for white)
                std::cout << (cell.nega == NEGA_WHITE ? "n " : "N ");
            } else if (cell.type == "poly") {
                // Print polyominos with P and show size
                int size = getPolySize(cell.polyshape);
                std::cout << "P" << size;
            } else if (cell.type == "ylop") {
                // Print ylops with Y and show size
                int size = getPolySize(cell.polyshape);
                std::cout << "Y" << size;
            } else {
                std::cout << "  ";
            }
        }
        std::cout << "\n";
    }
    std::cout << "\n";
}

// Recursive function to place polyominos in their region
bool Puzzle::placeShapesRecursively(const std::vector<std::pair<int, int>>& positions,
                                  std::vector<std::vector<int>>& grid,
                                  const std::vector<uint16_t>& shapes,
                                  const std::vector<std::pair<int, int>>& region,
                                  size_t shapeIndex) {
    // Base case: all shapes have been placed
    if (shapeIndex >= shapes.size()) {
        return true;
    }
    
    // Get the current shape to place
    uint16_t shape = shapes[shapeIndex];
    
    // Try all rotations of the shape
    std::vector<uint16_t> rotations = getRotations(shape | ROTATION_BIT);
    
    // Try placing the polyomino at each possible position in the region
    for (const auto& position : positions) {
        for (auto rotation : rotations) {
            // Generate cell coordinates for this shape based on rotation
            auto cells = polyominoFromPolyshape(rotation, false);
            
            // Try to place the shape at this position
            // Polys add +1 to cells, canceling out the -1
            if (tryPlacePolyshape(cells, position.first, position.second, grid, 1, region)) {
                // Recursively try to place the remaining shapes
                if (placeShapesRecursively(positions, grid, shapes, region, shapeIndex + 1)) {
                    return true;
                }
                
                // If placing remaining shapes failed, undo this placement
                tryPlacePolyshape(cells, position.first, position.second, grid, -1, region);
            }
        }
    }
    
    // No valid placement found for all shapes
    return false;
}


