// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SimulatorTypes.h"
#include "ZoneManager.generated.h"

class ATerrainZone;

/**
 * Central manager for all terrain zones in the world
 * Handles zone queries, management, and future dynamic zone creation
 */
UCLASS()
class SIMULATOR_API AZoneManager : public AActor
{
	GENERATED_BODY()

public:
	AZoneManager();

	virtual void BeginPlay() override;

public:
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

	// Refresh zone list (useful for dynamic zones later)
	void RefreshZoneList();
};
