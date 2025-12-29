// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "SimulatorTypes.h"
#include "BuildingManagerSubsystem.generated.h"

/**
 * Manager for all buildings in the world as a WorldSubsystem
 * Provides queries for finding buildings, managing construction, etc.
 */
UCLASS()
class SIMULATOR_API UBuildingManagerSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	// USubsystem implementation
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

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

	// Get all buildings
	UFUNCTION(BlueprintCallable, Category = "Building Manager")
	TArray<class ABaseBuilding*> GetAllBuildings() const { return AllBuildings; }

	// Register a new building (called when construction completes or building is spawned)
	UFUNCTION(BlueprintCallable, Category = "Building Manager")
	void RegisterBuilding(class ABaseBuilding* Building);

	// Unregister a building (called when building is destroyed)
	UFUNCTION(BlueprintCallable, Category = "Building Manager")
	void UnregisterBuilding(class ABaseBuilding* Building);

	// === Construction Management ===

	// Create a construction site for a new building
	UFUNCTION(BlueprintCallable, Category = "Building Manager|Construction")
	class AConstructionSite* CreateConstructionSite(
		FVector Location,
		FRotator Rotation,
		TSubclassOf<class ABaseBuilding> BuildingClass,
		EBuildingType BuildingType,
		float RequiredWork = 100.0f,
		int32 MaxWorkers = 5
	);

	// Get all active construction sites
	UFUNCTION(BlueprintCallable, Category = "Building Manager|Construction")
	TArray<class AConstructionSite*> GetAllConstructionSites() const;

	// Get nearest construction site
	UFUNCTION(BlueprintCallable, Category = "Building Manager|Construction")
	class AConstructionSite* GetNearestConstructionSite(FVector Location) const;

	// Cancel construction (destroys the construction site)
	UFUNCTION(BlueprintCallable, Category = "Building Manager|Construction")
	bool CancelConstruction(class AConstructionSite* Site);

protected:
	// Cache of all buildings in the world
	UPROPERTY()
	TArray<class ABaseBuilding*> AllBuildings;

	// Cache of all active construction sites
	UPROPERTY()
	TArray<class AConstructionSite*> ConstructionSites;

	// How often to refresh the building list (in seconds)
	float RefreshInterval;

	// Timer for periodic refresh
	FTimerHandle RefreshTimerHandle;

	// Periodic refresh function
	void PeriodicRefresh();

	// Refresh construction sites list
	void RefreshConstructionSites();
};
