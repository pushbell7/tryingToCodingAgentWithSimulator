// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "SimulatorTypes.h"
#include "ZoneManagerSubsystem.generated.h"

class AZoneGrid;

/**
 * Central manager for zone grid system
 * DEPRECATED: Zone system migrated to data-driven ZoneGrid
 * TODO: Refactor to ZoneGrid-based queries
 */
UCLASS()
class SIMULATOR_API UZoneManagerSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	// USubsystem implementation
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// Get zone grid in world
	UFUNCTION(BlueprintCallable, Category = "Zone Manager")
	AZoneGrid* GetZoneGrid() const;

	// Get zone type at location (uses ZoneGrid)
	UFUNCTION(BlueprintCallable, Category = "Zone Manager")
	ETerrainZone GetZoneTypeAtLocation(FVector Location) const;

protected:
	// Cached zone grid reference
	UPROPERTY()
	AZoneGrid* CachedZoneGrid;
};
