# Wave Function Collapse for Unreal Engine

A tile-based Wave Function Collapse (WFC) algorithm implementation in Unreal Engine C++, inspired by quantum mechanics principles and designed for procedural content generation.

## Overview

This implementation uses an edge-based matching system to generate coherent tile-based patterns. Each tile defines edge types for its four cardinal directions, and the algorithm ensures only compatible edges connect during generation.

## Features

- **Edge-Based Tile Matching**: Define tile compatibility through edge types rather than explicit adjacency lists
- **Constraint Propagation**: Automatic propagation of placement constraints to neighboring cells
- **Entropy-Based Collapse**: Selects cells with lowest entropy (fewest possibilities) for optimal generation
- **Validation System**: Built-in edge rule validation to catch configuration errors
- **Blueprint Integration**: Fully exposed to Blueprints for easy configuration

## Algorithm Workflow

1. **Initialization**: All cells start in superposition with all possible tile types
2. **Observation**: Find cell with lowest entropy and collapse it to a specific tile
3. **Propagation**: Update neighboring cells to respect edge compatibility constraints
4. **Iteration**: Repeat until all cells are collapsed or contradiction occurs

## Installation

1. Copy `WaveFunctionCollapseComponent.h` and `WaveFunctionCollapseComponent.cpp` to your project's Source directory
2. Add the component to any Actor in your level
3. Configure tile types and edge compatibility rules in the Details panel
4. Call `GenerateGrid()` to generate the pattern

## Configuration

### Tile Types

Each tile requires:
- **Mesh**: The static mesh to spawn
- **North/East/South/West Edge**: Edge type for each direction

```cpp
// Example tile configuration in Blueprint or C++
FTileType GrassTile;
GrassTile.Mesh = GrassMesh;
GrassTile.NorthEdge = ETileEdgeType::Type_A;
GrassTile.EastEdge = ETileEdgeType::Type_A;
GrassTile.SouthEdge = ETileEdgeType::Type_A;
GrassTile.WestEdge = ETileEdgeType::Type_A;
```

### Edge Compatibility

Define which edge types can connect:

```cpp
// In the Details panel or constructor
CompatibleEdges.Add(ETileEdgeType::Type_A, ETileEdgeType::Type_A);
CompatibleEdges.Add(ETileEdgeType::Type_B, ETileEdgeType::Type_B);
```

### Grid Settings

- **Grid Width**: Number of cells horizontally
- **Grid Height**: Number of cells vertically  
- **Tile Size**: Spacing between tiles in world units

## Usage Example

```cpp
// In Blueprint or C++
UWaveFunctionCollapseComponent* WFC = GetComponentByClass<UWaveFunctionCollapseComponent>();

// Configure grid
WFC->GridWidth = 20;
WFC->GridHeight = 20;
WFC->TileSize = 100.0f;

// Add tile types (configured with meshes and edge types)
WFC->TileTypes.Add(GrassTile);
WFC->TileTypes.Add(PathTile);
WFC->TileTypes.Add(WaterTile);

// Generate
WFC->GenerateGrid();
```

## Edge-Based vs Index-Based Approach

This implementation uses **edge-based matching** instead of index-based adjacency lists:

### Advantages

- **Intuitive**: Define what edges look like, not which tiles connect
- **Maintainable**: Adding new tiles doesn't require updating adjacency lists
- **Scalable**: Works efficiently with large tile sets
- **Self-documenting**: Edge types can have meaningful names (e.g., "Road", "Grass")
- **Efficient**: Simple enum comparisons instead of array lookups

### Example Comparison

```cpp
// Index-based (NOT used in this implementation)
struct FTileType {
    TArray<int32> AllowedNorth;  // Must list all compatible tile indices
    TArray<int32> AllowedEast;
    // ... requires updating when adding new tiles
};

// Edge-based (used in this implementation)
struct FTileType {
    ETileEdgeType NorthEdge;  // Just define the edge type
    ETileEdgeType EastEdge;
    // ... automatically compatible with any tile sharing edge type
};
```

## Technical Details

### Complexity
- **Time Complexity**: O(N²) where N = number of grid cells
- **Memory Usage**: O(N × T) where T = number of tile types
- **Max Iterations**: N × 10 (safety limit to prevent infinite loops)

### Core Components

**FCell**: Represents a grid cell with possible tile states
```cpp
struct FCell {
    TArray<int32> PossibleStates;  // Indices of possible tiles
    bool bIsCollapsed;              // Has cell been determined?
    int32 FinalState;               // Final tile index after collapse
};
```

**ETileEdgeType**: Enum defining edge types for tile matching

**FTileType**: Structure defining a tile's mesh and edge types

## API Reference

### Public Functions

- `GenerateGrid()`: Runs the WFC algorithm and spawns meshes
- `ValidateEdgeRules()`: Checks if edge compatibility rules are valid

### Public Properties

- `GridWidth/GridHeight`: Grid dimensions
- `TileTypes`: Array of available tile configurations
- `CompatibleEdges`: Map of compatible edge type pairs
- `TileSize`: World space size of each tile

### Private Functions

- `FindCellWithLowestEntropy()`: Finds next cell to collapse
- `CollapseCell()`: Randomly selects a state for a cell
- `PropagateConstraints()`: Updates neighbors after collapse
- `UpdateCellPossibilities()`: Filters cell states based on constraints
- `AreEdgesCompatible()`: Checks if two edge types can connect

## Common Issues

### Generation Fails or Reaches Max Iterations

- Check that edge compatibility rules are symmetric
- Ensure all edge types used by tiles have compatibility rules defined
- Verify tile set allows for complete grid coverage

### Tiles Don't Match Up Visually

- Confirm edge types are assigned correctly to tile directions
- Ensure meshes are modeled to match their edge type definitions

## Future Enhancements

- Weighted tile selection based on probability
- Support for forced tile placement at specific locations
- 3D grid generation
- Backtracking to resolve contradictions
- Performance optimizations for large grids
- Visual debugging of entropy values

## References

- [Original WFC Algorithm by Maxim Gumin](https://github.com/mxgmn/WaveFunctionCollapse)
- Based on concepts from quantum mechanics (superposition, observation, collapse)
- Influenced by constraint satisfaction and entropy minimization

## Applications

- **Procedural Level Generation**: Dungeons, maps, worlds
- **Texture Synthesis**: Seamless texture generation
- **Architecture**: Floor plans and building layouts
- **Terrain Generation**: Biome-coherent landscapes
- **Puzzle Games**: Valid puzzle layout generation

## License

MIT License - Feel free to use in your projects

## Author

Daniel Santos