// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SimulatorTypes.h"
#include "ZoneGridGenerator.generated.h"

/**
 * Grid-based Zone Generator
 * Editor utility for automatically generating TerrainZone grid
 *
 * Usage:
 * 1. Place this actor in the level
 * 2. Set grid parameters (GridSize, CellSize, etc.)
 * 3. Click "Generate Grid Zones" button in Details panel
 * 4. Zones will be automatically created in grid pattern
 */
UCLASS()
class SIMULATOR_API AZoneGridGenerator : public AActor
{
	GENERATED_BODY()

public:
	AZoneGridGenerator();

protected:
	virtual void BeginPlay() override;

public:
	// === Grid Settings ===

	// Grid dimensions (number of cells in each direction)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid Settings", meta = (ClampMin = "1", ClampMax = "200"))
	int32 GridSizeX = 10;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid Settings", meta = (ClampMin = "1", ClampMax = "200"))
	int32 GridSizeY = 10;

	// Size of each cell (in Unreal units)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid Settings", meta = (ClampMin = "100.0", ClampMax = "100000.0"))
	float CellSize = 5000.0f; // 50m x 50m cells

	// Starting position (bottom-left corner)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid Settings")
	FVector GridOrigin = FVector(0, 0, 0);

	// === Zone Type Settings ===

	// Default zone type for all cells
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zone Type")
	ETerrainZone DefaultZoneType = ETerrainZone::Farmland;

	// Use height-based auto zone type detection?
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zone Type")
	bool bAutoDetectZoneType = true;

	// Height thresholds for auto-detection (if enabled)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zone Type|Auto Detection", meta = (EditCondition = "bAutoDetectZoneType"))
	float WaterHeightMax = -100.0f; // Below this = Water

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zone Type|Auto Detection", meta = (EditCondition = "bAutoDetectZoneType"))
	float MountainHeightMin = 500.0f; // Above this = Mountain

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zone Type|Auto Detection", meta = (EditCondition = "bAutoDetectZoneType"))
	float ForestProbability = 0.3f; // 30% chance for forest on flat land

	// === Generation Options ===

	// Clear existing zones before generating?
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation Options")
	bool bClearExistingZones = true;

	// Show debug visualization?
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation Options")
	bool bShowDebugVisualization = true;

	// Auto-register zones with ZoneManager?
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation Options")
	bool bAutoRegisterZones = true;

	// === Generation ===

#if WITH_EDITOR
	// Generate grid zones (Editor-only button)
	UFUNCTION(CallInEditor, Category = "Generation")
	void GenerateGridZones();

	// Clear all generated zones
	UFUNCTION(CallInEditor, Category = "Generation")
	void ClearGeneratedZones();

	// Preview grid (draws debug lines)
	UFUNCTION(CallInEditor, Category = "Generation")
	void PreviewGrid();
#endif

	// Get zone type for a specific cell based on height
	UFUNCTION(BlueprintCallable, Category = "Grid Generator")
	ETerrainZone DetermineZoneType(FVector Location, float Height);

	// Get cell center position for grid coordinates
	UFUNCTION(BlueprintCallable, Category = "Grid Generator")
	FVector GetCellCenter(int32 X, int32 Y) const;

	// Get terrain height at location
	UFUNCTION(BlueprintCallable, Category = "Grid Generator")
	float GetTerrainHeight(FVector Location) const;

protected:
	// Array of generated zones (for cleanup)
	UPROPERTY()
	TArray<class ATerrainZone*> GeneratedZones;

	// Spawn a single zone cell
	class ATerrainZone* SpawnZoneCell(int32 GridX, int32 GridY, ETerrainZone ZoneType);

	// Clear all zones in GeneratedZones array
	void ClearZones();
};
