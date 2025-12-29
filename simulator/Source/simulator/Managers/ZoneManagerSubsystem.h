// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "SimulatorTypes.h"
#include "ZoneManagerSubsystem.generated.h"

class ATerrainZone;

/**
 * Central manager for all terrain zones in the world as a WorldSubsystem
 * Handles zone queries, management, and future dynamic zone creation
 */
UCLASS()
class SIMULATOR_API UZoneManagerSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	// USubsystem implementation
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// Refresh zone list (useful for dynamic zones later)
	UFUNCTION(BlueprintCallable, Category = "Zone Manager")
	void RefreshZoneList();

	// Get all zones of a specific type
	UFUNCTION(BlueprintCallable, Category = "Zone Manager")
	TArray<ATerrainZone*> GetZonesByType(ETerrainZone ZoneType) const;

	// Get nearest zone of a specific type from a location
	UFUNCTION(BlueprintCallable, Category = "Zone Manager")
	ATerrainZone* GetNearestZone(FVector Location, ETerrainZone ZoneType) const;

	// Get all zones within a radius
	UFUNCTION(BlueprintCallable, Category = "Zone Manager")
	TArray<ATerrainZone*> GetZonesWithinRadius(FVector Location, float Radius) const;

	// Get all zones in the world
	UFUNCTION(BlueprintCallable, Category = "Zone Manager")
	TArray<ATerrainZone*> GetAllZones() const { return AllZones; }

	// Get zone that contains a specific location
	UFUNCTION(BlueprintCallable, Category = "Zone Manager")
	ATerrainZone* GetZoneAtLocation(FVector Location) const;

	// Get total number of zones
	UFUNCTION(BlueprintCallable, Category = "Zone Manager")
	int32 GetZoneCount() const { return AllZones.Num(); }

protected:
	// All zones in the world
	UPROPERTY()
	TArray<ATerrainZone*> AllZones;
};
