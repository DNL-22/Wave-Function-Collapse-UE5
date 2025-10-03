// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "WaveFunctionCollapseComponent.generated.h"

// Enum to represent connection types on tile edges
UENUM(BlueprintType)
enum class ETileEdgeType : uint8
{
    Type_A UMETA(DisplayName = "Type A"),
    Type_B UMETA(DisplayName = "Type B"),
    Type_C UMETA(DisplayName = "Type C"),
    Type_D UMETA(DisplayName = "Type D")
    // Add more types as needed
};

// Structure to represent a tile type with its edge types
USTRUCT(BlueprintType)
struct WFC_API FTileType
{
    GENERATED_USTRUCT_BODY()

    // The mesh to use for this tile
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Tile Properties")
    UStaticMesh* Mesh;

    // Edge types for each direction (North, East, South, West)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Edge Types")
    ETileEdgeType NorthEdge;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Edge Types")
    ETileEdgeType EastEdge;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Edge Types")
    ETileEdgeType SouthEdge;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Edge Types")
    ETileEdgeType WestEdge;

    FTileType()
    {
        Mesh = nullptr;
        NorthEdge = ETileEdgeType::Type_A;
        EastEdge = ETileEdgeType::Type_A;
        SouthEdge = ETileEdgeType::Type_A;
        WestEdge = ETileEdgeType::Type_A;
    }
};

// Represents a cell in the grid that can be collapsed to a specific tile type
USTRUCT(BlueprintType)
struct WFC_API FCell 
{
    GENERATED_USTRUCT_BODY()

    // Possible states (tile types) this cell can have
    UPROPERTY(VisibleAnywhere, Category="Cell Properties")
    TArray<int32> PossibleStates;

    // Is this cell collapsed to a single state?
    UPROPERTY(VisibleAnywhere, Category="Cell Properties")
    bool bIsCollapsed;

    // The final state after collapse (tile type index)
    UPROPERTY(VisibleAnywhere, Category="Cell Properties")
    int32 FinalState;

    // Constructor with default values
    FCell() 
    {
        bIsCollapsed = false;
        FinalState = -1;
    }
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class WFC_API UWaveFunctionCollapseComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UWaveFunctionCollapseComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Starts the WFC algorithm and generates the grid
    UFUNCTION(BlueprintCallable, Category = "WaveFunctionCollapse")
    void GenerateGrid();

    // Define the grid dimensions
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WaveFunctionCollapse")
    int32 GridWidth = 10;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WaveFunctionCollapse")
    int32 GridHeight = 10;

    // The tile types available
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WaveFunctionCollapse")
    TArray<FTileType> TileTypes;

    // Edge compatibility rules - defines which edge types can connect
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WaveFunctionCollapse")
    TMap<ETileEdgeType, ETileEdgeType> CompatibleEdges;

    // Spacing between cells
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WaveFunctionCollapse")
    float TileSize = 100.f;

    // Validate that the edge compatibility rules are properly set up
    UFUNCTION(BlueprintCallable, Category = "WaveFunctionCollapse")
    bool ValidateEdgeRules();
	
private:
    // The grid of cells
    TArray<FCell> Grid;

    // Find the cell with the lowest entropy (fewest possible states)
    int32 FindCellWithLowestEntropy();

    // Collapse a single cell to a definite state
    void CollapseCell(int32 CellIndex);

    // Propagate constraints after a cell has been collapsed
    void PropagateConstraints(int32 CellIndex);

    // Update possible states of a neighboring cell
    bool UpdateCellPossibilities(int32 CellIndex, const TArray<int32>& AllowedStates);

    // Check if edge types are compatible
    bool AreEdgesCompatible(ETileEdgeType Edge1, ETileEdgeType Edge2);

    // Get all tile indices that have a specific edge type in a specific direction
    TArray<int32> GetTilesWithEdgeType(ETileEdgeType EdgeType, const FString& Direction);

    // Check if all cells have been collapsed
    bool IsGridFullyCollapsed();

    // Convert a grid index to a 2D position
    void IndexToXY(int32 Index, int32& OutX, int32& OutY);

    // Convert a 2D position to a grid index
    int32 XYToIndex(int32 X, int32 Y);

    // Spawn the actual tile meshes
    void SpawnTileMeshes();
};