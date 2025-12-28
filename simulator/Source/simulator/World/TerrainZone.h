// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Volume.h"
#include "SimulatorTypes.h"
#include "TerrainZone.generated.h"

/**
 * Terrain zone volume for resource management
 * Place this in the level to define different terrain types
 */
UCLASS()
class SIMULATOR_API ATerrainZone : public AVolume
{
	GENERATED_BODY()

public:
	ATerrainZone();

	virtual void BeginPlay() override;

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

public:
	// Zone type (Farmland, Forest, Mountain, etc)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zone")
	ETerrainZone ZoneType;

	// Resource richness (0.0 = depleted, 1.0 = very rich)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zone", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float ResourceRichness;

	// Fertility for farming (only used for Farmland/Pasture)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zone", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float Fertility;

	// Display name for this zone
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zone")
	FString ZoneName;

	// Check if an actor is inside this zone
	UFUNCTION(BlueprintCallable, Category = "Zone")
	bool IsActorInZone(AActor* Actor) const;

	// Get all actors currently in this zone
	UFUNCTION(BlueprintCallable, Category = "Zone")
	TArray<AActor*> GetActorsInZone(TSubclassOf<AActor> ActorClass) const;

	// Get zone center location
	UFUNCTION(BlueprintCallable, Category = "Zone")
	FVector GetZoneCenter() const;

	// Get resource type produced by this zone
	UFUNCTION(BlueprintCallable, Category = "Zone")
	EResourceType GetProducedResourceType() const;

	// Attempt to gather resources from this zone (returns amount gathered)
	UFUNCTION(BlueprintCallable, Category = "Zone")
	int32 GatherResources(int32 RequestedAmount);

	// Check if this zone can produce resources
	UFUNCTION(BlueprintCallable, Category = "Zone")
	bool CanProduceResources() const;

protected:
	// Cache of actors currently in zone (updated periodically)
	UPROPERTY()
	TArray<AActor*> CachedActorsInZone;
};
