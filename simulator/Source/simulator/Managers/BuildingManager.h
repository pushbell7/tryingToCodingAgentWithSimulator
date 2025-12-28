// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SimulatorTypes.h"
#include "BuildingManager.generated.h"

/**
 * Manager for all buildings in the world
 * Provides queries for finding buildings, managing construction, etc.
 */
UCLASS()
class SIMULATOR_API ABuildingManager : public AActor
{
	GENERATED_BODY()

public:
	ABuildingManager();

protected:
	virtual void BeginPlay() override;

public:
	// Find all buildings in the world
	UFUNCTION(BlueprintCallable, Category = "Building Manager")
	void RefreshBuildingList();

	// Get all buildings of a specific type
	UFUNCTION(BlueprintCallable, Category = "Building Manager")
	TArray<class ABaseBuilding*> GetBuildingsByType(EBuildingType BuildingType) const;

	// Get nearest building of a specific type
	UFUNCTION(BlueprintCallable, Category = "Building Manager")
	class ABaseBuilding* GetNearestBuilding(FVector Location, EBuildingType BuildingType) const;

	// Get all buildings within a radius
	UFUNCTION(BlueprintCallable, Category = "Building Manager")
	TArray<class ABaseBuilding*> GetBuildingsWithinRadius(FVector Location, float Radius) const;

	// Get nearest storage building (Warehouse or Granary)
	UFUNCTION(BlueprintCallable, Category = "Building Manager")
	class ABaseBuilding* GetNearestStorageBuilding(FVector Location) const;

	// Get all storage buildings
	UFUNCTION(BlueprintCallable, Category = "Building Manager")
	TArray<class ABaseBuilding*> GetAllStorageBuildings() const;

	// Get nearest storage building that can accept resources
	UFUNCTION(BlueprintCallable, Category = "Building Manager")
	class ABaseBuilding* GetNearestAvailableStorage(FVector Location) const;

	// Get total building count
	UFUNCTION(BlueprintCallable, Category = "Building Manager")
	int32 GetBuildingCount() const;

	// Get building count by type
	UFUNCTION(BlueprintCallable, Category = "Building Manager")
	int32 GetBuildingCountByType(EBuildingType BuildingType) const;

protected:
	// Cache of all buildings in the world
	UPROPERTY()
	TArray<class ABaseBuilding*> AllBuildings;

	// How often to refresh the building list (in seconds)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Building Manager")
	float RefreshInterval;

	// Timer for periodic refresh
	FTimerHandle RefreshTimerHandle;

	// Periodic refresh function
	void PeriodicRefresh();
};
