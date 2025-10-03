// Fill out your copyright notice in the Description page of Project Settings.


#include "WaveFunctionCollapseComponent.h"
#include "Engine/World.h"

// Sets default values for this component's properties
UWaveFunctionCollapseComponent::UWaveFunctionCollapseComponent()
{
    // Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
    // off to improve performance if you don't need them.
    PrimaryComponentTick.bCanEverTick = true;

    // Initialize default edge compatibility
    CompatibleEdges.Add(ETileEdgeType::Type_A, ETileEdgeType::Type_A);
    CompatibleEdges.Add(ETileEdgeType::Type_B, ETileEdgeType::Type_B);
    CompatibleEdges.Add(ETileEdgeType::Type_C, ETileEdgeType::Type_C);
    CompatibleEdges.Add(ETileEdgeType::Type_D, ETileEdgeType::Type_D);
}


// Called when the game starts
void UWaveFunctionCollapseComponent::BeginPlay()
{
    Super::BeginPlay();

    ValidateEdgeRules();
    GenerateGrid();
}


// Called every frame
void UWaveFunctionCollapseComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UWaveFunctionCollapseComponent::GenerateGrid()
{
    // Validate edge rules before generating
    if (!ValidateEdgeRules())
    {
        UE_LOG(LogTemp, Error, TEXT("Wave Function Collapse failed: Invalid edge compatibility rules"));
        return;
    }

    // Initialize grid
    Grid.Empty();
    Grid.SetNum(GridWidth * GridHeight);

    // Set all cells to have all possible states initially
    for (FCell& Cell : Grid)
    {
        Cell.bIsCollapsed = false;
        Cell.PossibleStates.Empty();

        // Add all tile types as possible states
        for (int32 i = 0; i < TileTypes.Num(); ++i)
        {
            Cell.PossibleStates.Add(i);
        }
    }

    // Run the WFC algorithm until the grid is fully collapsed
    int32 MaxIterations = GridWidth * GridHeight * 10; // Safety limit to prevent infinite loops
    int32 IterationCount = 0;

    while (!IsGridFullyCollapsed() && IterationCount < MaxIterations)
    {
        // Find the cell with the lowest entropy
        int32 CellToCollapse = FindCellWithLowestEntropy();

        if (CellToCollapse == -1)
            break;  // No valid cells left to collapse

        // Collapse the cell
        CollapseCell(CellToCollapse);

        // Propagate constraints
        PropagateConstraints(CellToCollapse);

        IterationCount++;
    }

    if (IterationCount >= MaxIterations)
    {
        UE_LOG(LogTemp, Warning, TEXT("Wave Function Collapse reached max iterations (%d). Grid may be incomplete."), MaxIterations);
    }

    // Spawn the meshes
    SpawnTileMeshes();
}

int32 UWaveFunctionCollapseComponent::FindCellWithLowestEntropy()
{
    int32 LowestEntropyIndex = -1;
    int32 LowestEntropy = TNumericLimits<int32>::Max();

    // Find uncollapsed cell with fewest possible states
    for (int32 i = 0; i < Grid.Num(); ++i)
    {
        const FCell& Cell = Grid[i];

        if (!Cell.bIsCollapsed && Cell.PossibleStates.Num() > 0)
        {
            if (Cell.PossibleStates.Num() < LowestEntropy)
            {
                LowestEntropy = Cell.PossibleStates.Num();
                LowestEntropyIndex = i;
            }
        }
    }

    return LowestEntropyIndex;
}

void UWaveFunctionCollapseComponent::CollapseCell(int32 CellIndex)
{
    if (CellIndex < 0 || CellIndex >= Grid.Num())
        return;

    FCell& Cell = Grid[CellIndex];

    if (Cell.bIsCollapsed || Cell.PossibleStates.Num() == 0)
        return;

    // Choose a random state from the possible states
    int32 RandomIndex = FMath::RandRange(0, Cell.PossibleStates.Num() - 1);
    int32 ChosenState = Cell.PossibleStates[RandomIndex];

    // Collapse the cell to this state
    Cell.PossibleStates.Empty();
    Cell.PossibleStates.Add(ChosenState);
    Cell.FinalState = ChosenState;
    Cell.bIsCollapsed = true;
}

void UWaveFunctionCollapseComponent::PropagateConstraints(int32 CellIndex)
{
    if (CellIndex < 0 || CellIndex >= Grid.Num())
        return;

    // Create a queue for propagation
    TArray<int32> PropagationQueue;
    PropagationQueue.Add(CellIndex);

    // Process the queue
    while (PropagationQueue.Num() > 0)
    {
        int32 CurrentCellIndex = PropagationQueue[0];
        PropagationQueue.RemoveAt(0);

        int32 X, Y;
        IndexToXY(CurrentCellIndex, X, Y);

        const FCell& CurrentCell = Grid[CurrentCellIndex];

        // Process North neighbor
        if (Y > 0)
        {
            int32 NorthIndex = XYToIndex(X, Y - 1);

            // Get allowed north neighbors for current cell's possible states
            TArray<int32> AllowedStatesForNorth;

            for (int32 StateIndex : CurrentCell.PossibleStates)
            {
                // Get the north edge type of the current tile
                ETileEdgeType CurrentNorthEdge = TileTypes[StateIndex].NorthEdge;

                // Find all tiles that have a compatible south edge
                for (int32 i = 0; i < TileTypes.Num(); ++i)
                {
                    ETileEdgeType PotentialSouthEdge = TileTypes[i].SouthEdge;
                    if (AreEdgesCompatible(CurrentNorthEdge, PotentialSouthEdge))
                    {
                        AllowedStatesForNorth.AddUnique(i);
                    }
                }
            }

            // Update the neighbor's possible states based on constraint
            if (UpdateCellPossibilities(NorthIndex, AllowedStatesForNorth))
            {
                PropagationQueue.AddUnique(NorthIndex);
            }
        }

        // Process East neighbor
        if (X < GridWidth - 1)
        {
            int32 EastIndex = XYToIndex(X + 1, Y);

            // Get allowed east neighbors for current cell's possible states
            TArray<int32> AllowedStatesForEast;

            for (int32 StateIndex : CurrentCell.PossibleStates)
            {
                // Get the east edge type of the current tile
                ETileEdgeType CurrentEastEdge = TileTypes[StateIndex].EastEdge;

                // Find all tiles that have a compatible west edge
                for (int32 i = 0; i < TileTypes.Num(); ++i)
                {
                    ETileEdgeType PotentialWestEdge = TileTypes[i].WestEdge;
                    if (AreEdgesCompatible(CurrentEastEdge, PotentialWestEdge))
                    {
                        AllowedStatesForEast.AddUnique(i);
                    }
                }
            }

            // Update the neighbor's possible states based on constraint
            if (UpdateCellPossibilities(EastIndex, AllowedStatesForEast))
            {
                PropagationQueue.AddUnique(EastIndex);
            }
        }

        // Process South neighbor
        if (Y < GridHeight - 1)
        {
            int32 SouthIndex = XYToIndex(X, Y + 1);

            // Get allowed south neighbors for current cell's possible states
            TArray<int32> AllowedStatesForSouth;

            for (int32 StateIndex : CurrentCell.PossibleStates)
            {
                // Get the south edge type of the current tile
                ETileEdgeType CurrentSouthEdge = TileTypes[StateIndex].SouthEdge;

                // Find all tiles that have a compatible north edge
                for (int32 i = 0; i < TileTypes.Num(); ++i)
                {
                    ETileEdgeType PotentialNorthEdge = TileTypes[i].NorthEdge;
                    if (AreEdgesCompatible(CurrentSouthEdge, PotentialNorthEdge))
                    {
                        AllowedStatesForSouth.AddUnique(i);
                    }
                }
            }

            // Update the neighbor's possible states based on constraint
            if (UpdateCellPossibilities(SouthIndex, AllowedStatesForSouth))
            {
                PropagationQueue.AddUnique(SouthIndex);
            }
        }

        // Process West neighbor
        if (X > 0)
        {
            int32 WestIndex = XYToIndex(X - 1, Y);

            // Get allowed west neighbors for current cell's possible states
            TArray<int32> AllowedStatesForWest;

            for (int32 StateIndex : CurrentCell.PossibleStates)
            {
                // Get the west edge type of the current tile
                ETileEdgeType CurrentWestEdge = TileTypes[StateIndex].WestEdge;

                // Find all tiles that have a compatible east edge
                for (int32 i = 0; i < TileTypes.Num(); ++i)
                {
                    ETileEdgeType PotentialEastEdge = TileTypes[i].EastEdge;
                    if (AreEdgesCompatible(CurrentWestEdge, PotentialEastEdge))
                    {
                        AllowedStatesForWest.AddUnique(i);
                    }
                }
            }

            // Update the neighbor's possible states based on constraint
            if (UpdateCellPossibilities(WestIndex, AllowedStatesForWest))
            {
                PropagationQueue.AddUnique(WestIndex);
            }
        }
    }
}

bool UWaveFunctionCollapseComponent::UpdateCellPossibilities(int32 CellIndex, const TArray<int32>& AllowedStates)
{
    if (CellIndex < 0 || CellIndex >= Grid.Num())
        return false;

    FCell& Cell = Grid[CellIndex];

    if (Cell.bIsCollapsed)
        return false;

    int32 PreviousCount = Cell.PossibleStates.Num();

    // Create a filtered list of possible states based on constraints
    TArray<int32> NewPossibleStates;

    for (int32 State : Cell.PossibleStates)
    {
        if (AllowedStates.Contains(State))
        {
            NewPossibleStates.Add(State);
        }
    }

    // Update the cell's possible states if the new list isn't empty
    if (NewPossibleStates.Num() > 0)
    {
        Cell.PossibleStates = NewPossibleStates;

        // If the cell now has only one possible state, collapse it
        if (Cell.PossibleStates.Num() == 1)
        {
            Cell.FinalState = Cell.PossibleStates[0];
            Cell.bIsCollapsed = true;
        }
    }

    // Return true if the possible states changed
    return PreviousCount != Cell.PossibleStates.Num();
}

bool UWaveFunctionCollapseComponent::AreEdgesCompatible(ETileEdgeType Edge1, ETileEdgeType Edge2)
{
    // If this edge type has a defined compatible edge, check against it
    if (const ETileEdgeType* CompatibleEdge = CompatibleEdges.Find(Edge1))
    {
        return *CompatibleEdge == Edge2;
    }

    return false;
}

TArray<int32> UWaveFunctionCollapseComponent::GetTilesWithEdgeType(ETileEdgeType EdgeType, const FString& Direction)
{
    TArray<int32> MatchingTiles;

    for (int32 i = 0; i < TileTypes.Num(); ++i)
    {
        const FTileType& Tile = TileTypes[i];

        if (Direction == "North" && Tile.NorthEdge == EdgeType)
            MatchingTiles.Add(i);
        else if (Direction == "East" && Tile.EastEdge == EdgeType)
            MatchingTiles.Add(i);
        else if (Direction == "South" && Tile.SouthEdge == EdgeType)
            MatchingTiles.Add(i);
        else if (Direction == "West" && Tile.WestEdge == EdgeType)
            MatchingTiles.Add(i);
    }

    return MatchingTiles;
}

bool UWaveFunctionCollapseComponent::ValidateEdgeRules()
{
    if (TileTypes.Num() == 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("No tile types defined"));
        return false;
    }

    bool bRulesAreValid = true;

    // Check that all edge types have compatibility rules
    for (int32 i = 0; i < TileTypes.Num(); ++i)
    {
        const FTileType& Tile = TileTypes[i];

        // Check North edge
        if (!CompatibleEdges.Contains(Tile.NorthEdge))
        {
            UE_LOG(LogTemp, Warning, TEXT("Tile %d has North edge type with no compatibility rule"), i);
            bRulesAreValid = false;
        }

        // Check East edge
        if (!CompatibleEdges.Contains(Tile.EastEdge))
        {
            UE_LOG(LogTemp, Warning, TEXT("Tile %d has East edge type with no compatibility rule"), i);
            bRulesAreValid = false;
        }

        // Check South edge
        if (!CompatibleEdges.Contains(Tile.SouthEdge))
        {
            UE_LOG(LogTemp, Warning, TEXT("Tile %d has South edge type with no compatibility rule"), i);
            bRulesAreValid = false;
        }

        // Check West edge
        if (!CompatibleEdges.Contains(Tile.WestEdge))
        {
            UE_LOG(LogTemp, Warning, TEXT("Tile %d has West edge type with no compatibility rule"), i);
            bRulesAreValid = false;
        }
    }

    // Check for symmetry in compatibility rules
    for (const auto& Pair : CompatibleEdges)
    {
        ETileEdgeType Edge1 = Pair.Key;
        ETileEdgeType Edge2 = Pair.Value;

        // Check if Edge2 has Edge1 as its compatible edge
        const ETileEdgeType* ReverseCompatible = CompatibleEdges.Find(Edge2);
        if (!ReverseCompatible || *ReverseCompatible != Edge1)
        {
            UE_LOG(LogTemp, Warning, TEXT("Edge compatibility is not symmetric: %d -> %d, but %d -> %d"),
                static_cast<int32>(Edge1), static_cast<int32>(Edge2),
                static_cast<int32>(Edge2), ReverseCompatible ? static_cast<int32>(*ReverseCompatible) : -1);
            bRulesAreValid = false;
        }
    }

    return bRulesAreValid;
}

bool UWaveFunctionCollapseComponent::IsGridFullyCollapsed()
{
    for (const FCell& Cell : Grid)
    {
        if (!Cell.bIsCollapsed)
            return false;
    }

    return true;
}

void UWaveFunctionCollapseComponent::IndexToXY(int32 Index, int32& OutX, int32& OutY)
{
    OutX = Index % GridWidth;
    OutY = Index / GridWidth;
}

int32 UWaveFunctionCollapseComponent::XYToIndex(int32 X, int32 Y)
{
    return Y * GridWidth + X;
}

void UWaveFunctionCollapseComponent::SpawnTileMeshes()
{
    UWorld* World = GetWorld();
    if (!World)
        return;

    // Get the component's owner location as origin
    FVector Origin = GetOwner()->GetActorLocation();

    // Spawn a static mesh for each cell
    for (int32 i = 0; i < Grid.Num(); ++i)
    {
        const FCell& Cell = Grid[i];

        if (Cell.bIsCollapsed && Cell.FinalState >= 0 && Cell.FinalState < TileTypes.Num())
        {
            int32 X, Y;
            IndexToXY(i, X, Y);

            // Calculate the position of this tile
            FVector Position = Origin + FVector(X * TileSize, Y * TileSize, 0);

            // Get the mesh for this tile type
            UStaticMesh* TileMesh = TileTypes[Cell.FinalState].Mesh;

            if (TileMesh)
            {
                // Spawn the static mesh component
                UStaticMeshComponent* MeshComponent = NewObject<UStaticMeshComponent>(GetOwner());
                MeshComponent->SetStaticMesh(TileMesh);
                MeshComponent->SetRelativeLocation(Position);
                MeshComponent->RegisterComponent();
            }
        }
    }
}