// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SimulatorTypes.h"
#include "ZoneGrid.generated.h"

/**
 * Single cell data in the zone grid
 * Stores zone type, position, and optional properties
 */
USTRUCT(BlueprintType)
struct FZoneCellData
{
	GENERATED_BODY()

	// Zone type for this cell
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zone Cell")
	ETerrainZone ZoneType = ETerrainZone::Farmland;

	// Grid coordinates (X, Y)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Zone Cell")
	FIntPoint GridCoords = FIntPoint(0, 0);

	// World position (center of cell)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Zone Cell")
	FVector WorldPosition = FVector::ZeroVector;

	// Optional: Resource richness (0.0 - 1.0)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zone Cell", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float ResourceRichness = 1.0f;

	FZoneCellData() = default;

	FZoneCellData(ETerrainZone InZoneType, FIntPoint InGridCoords, FVector InWorldPosition)
		: ZoneType(InZoneType)
		, GridCoords(InGridCoords)
		, WorldPosition(InWorldPosition)
		, ResourceRichness(1.0f)
	{}
};

/**
 * Data-driven Zone Grid System
 * Single actor manages entire zone grid as a data structure
 *
 * Features:
 * - TArray-based cell storage (efficient for large maps)
 * - Editor visualization with color-coded cells
 * - Paint-brush editing capability
 * - Automatic world position calculation
 * - Building placement constraint queries
 */
UCLASS()
class SIMULATOR_API AZoneGrid : public AActor
{
	GENERATED_BODY()

public:
	AZoneGrid();

protected:
	virtual void BeginPlay() override;

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

public:
	// === Grid Settings ===

	// Grid dimensions (number of cells)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid Settings", meta = (ClampMin = "1", ClampMax = "500"))
	int32 GridSizeX = 100;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid Settings", meta = (ClampMin = "1", ClampMax = "500"))
	int32 GridSizeY = 100;

	// Size of each cell in world units (cm)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid Settings", meta = (ClampMin = "100.0", ClampMax = "100000.0"))
	float CellSize = 5000.0f; // 50m x 50m cells

	// Grid origin (bottom-left corner)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid Settings")
	FVector GridOrigin = FVector(0, 0, 0);

	// === Zone Data ===

	// All cell data (GridSizeX * GridSizeY elements)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zone Data")
	TArray<FZoneCellData> ZoneCells;

	// === Auto-Generation Settings ===

	// Default zone type for initialization
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Auto-Generation")
	ETerrainZone DefaultZoneType = ETerrainZone::Farmland;

	// Use height-based auto zone type detection?
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Auto-Generation")
	bool bAutoDetectZoneType = true;

	// Height thresholds for auto-detection
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Auto-Generation|Thresholds", meta = (EditCondition = "bAutoDetectZoneType"))
	float WaterHeightMax = -100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Auto-Generation|Thresholds", meta = (EditCondition = "bAutoDetectZoneType"))
	float MountainHeightMin = 500.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Auto-Generation|Thresholds", meta = (EditCondition = "bAutoDetectZoneType"))
	float ForestProbability = 0.3f;

	// === Editor Visualization ===

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visualization")
	bool bShowGridVisualization = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visualization")
	bool bShowCellBorders = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visualization")
	float VisualizationHeight = 100.0f; // Height offset for drawing

	// === Generation Functions ===

#if WITH_EDITOR
	// Initialize grid with cells
	UFUNCTION(CallInEditor, Category = "Generation")
	void InitializeGrid();

	// Clear all cells
	UFUNCTION(CallInEditor, Category = "Generation")
	void ClearGrid();

	// Auto-generate zone types based on terrain height
	UFUNCTION(CallInEditor, Category = "Generation")
	void AutoGenerateZoneTypes();

	// Visualize grid in editor
	UFUNCTION(CallInEditor, Category = "Generation")
	void VisualizeGrid();
#endif

	// === Query Functions (Runtime & Editor) ===

	// Get zone type at world location
	UFUNCTION(BlueprintCallable, Category = "Zone Grid")
	ETerrainZone GetZoneTypeAtLocation(FVector WorldLocation) const;

	// Get zone type at grid coordinates
	UFUNCTION(BlueprintCallable, Category = "Zone Grid")
	ETerrainZone GetZoneTypeAtGridCoords(int32 X, int32 Y) const;

	// Get cell data at location
	UFUNCTION(BlueprintCallable, Category = "Zone Grid")
	bool GetCellAtLocation(FVector WorldLocation, FZoneCellData& OutCell) const;

	// Get cell data at grid coordinates
	UFUNCTION(BlueprintCallable, Category = "Zone Grid")
	bool GetCellAtGridCoords(int32 X, int32 Y, FZoneCellData& OutCell) const;

	// Check if location is valid for building type
	UFUNCTION(BlueprintCallable, Category = "Zone Grid")
	bool CanBuildAtLocation(FVector WorldLocation, ETerrainZone RequiredZoneType) const;

	// Get grid coordinates from world location
	UFUNCTION(BlueprintCallable, Category = "Zone Grid")
	FIntPoint WorldToGridCoords(FVector WorldLocation) const;

	// Get world position from grid coordinates
	UFUNCTION(BlueprintCallable, Category = "Zone Grid")
	FVector GridCoordsToWorld(int32 X, int32 Y) const;

	// Get total number of cells
	UFUNCTION(BlueprintCallable, Category = "Zone Grid")
	int32 GetTotalCells() const { return GridSizeX * GridSizeY; }

	// === Editor Paint Functions ===

	// Paint zone type in area (used by Editor Mode Plugin)
	void PaintZoneArea(FVector WorldLocation, int32 BrushRadius, ETerrainZone ZoneType);

protected:
	// Convert 2D grid coords to 1D array index
	int32 GridCoordsToIndex(int32 X, int32 Y) const;

	// Check if grid coordinates are valid
	bool IsValidGridCoords(int32 X, int32 Y) const;

	// Get terrain height at location (for auto-generation)
	float GetTerrainHeight(FVector Location) const;

	// Determine zone type from height (for auto-generation)
	ETerrainZone DetermineZoneType(FVector Location, float Height) const;

	// Get color for zone type (for visualization)
	FColor GetZoneColor(ETerrainZone ZoneType) const;
};
