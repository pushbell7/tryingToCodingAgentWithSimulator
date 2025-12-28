// Copyright Epic Games, Inc. All Rights Reserved.

#include "TerrainZone.h"
#include "Components/BrushComponent.h"
#include "Engine/CollisionProfile.h"
#include "DrawDebugHelpers.h"
#include "BaseVillager.h"

ATerrainZone::ATerrainZone()
{
	PrimaryActorTick.bCanEverTick = false;

	// Default values
	ZoneType = ETerrainZone::Farmland;
	ResourceRichness = 0.5f;
	Fertility = 0.5f;
	ZoneName = TEXT("Terrain Zone");

	// Worker capacity defaults
	MaxWorkers = 5; // Default: 5 workers per zone
	CurrentWorkers = 0;
}

void ATerrainZone::BeginPlay()
{
	Super::BeginPlay();

	// Log zone info and set debug color
	FString ZoneTypeName;
	FColor DebugColor;
	switch (ZoneType)
	{
	case ETerrainZone::Farmland:
		ZoneTypeName = TEXT("Farmland");
		DebugColor = FColor::Yellow;
		break;
	case ETerrainZone::Pasture:
		ZoneTypeName = TEXT("Pasture");
		DebugColor = FColor::Green;
		break;
	case ETerrainZone::Forest:
		ZoneTypeName = TEXT("Forest");
		DebugColor = FColor(34, 139, 34); // Forest green
		break;
	case ETerrainZone::Mountain:
		ZoneTypeName = TEXT("Mountain");
		DebugColor = FColor(128, 128, 128); // Gray
		break;
	case ETerrainZone::Water:
		ZoneTypeName = TEXT("Water");
		DebugColor = FColor::Cyan;
		break;
	case ETerrainZone::Settlement:
		ZoneTypeName = TEXT("Settlement");
		DebugColor = FColor::Orange;
		break;
	default:
		ZoneTypeName = TEXT("Unknown");
		DebugColor = FColor::White;
		break;
	}

	UE_LOG(LogTemp, Log, TEXT("TerrainZone '%s' initialized - Type: %s, Richness: %.2f, Fertility: %.2f"),
		*ZoneName, *ZoneTypeName, ResourceRichness, Fertility);

	// Draw debug visualization for Play mode
	if (GetWorld())
	{
		FVector Origin, BoxExtent;
		GetActorBounds(true, Origin, BoxExtent);

		DrawDebugBox(GetWorld(), Origin, BoxExtent, DebugColor, true, -1.0f, 0, 10.0f);

		UE_LOG(LogTemp, Log, TEXT("TerrainZone '%s' debug box drawn at %s with extent %s"),
			*ZoneName, *Origin.ToString(), *BoxExtent.ToString());
	}
}

#if WITH_EDITOR
void ATerrainZone::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	// Auto-name zone based on type
	FName PropertyName = (PropertyChangedEvent.Property != nullptr) ? PropertyChangedEvent.Property->GetFName() : NAME_None;
	if (PropertyName == GET_MEMBER_NAME_CHECKED(ATerrainZone, ZoneType))
	{
		switch (ZoneType)
		{
		case ETerrainZone::Farmland:
			ZoneName = TEXT("Farmland Zone");
			break;
		case ETerrainZone::Pasture:
			ZoneName = TEXT("Pasture Zone");
			break;
		case ETerrainZone::Forest:
			ZoneName = TEXT("Forest Zone");
			break;
		case ETerrainZone::Mountain:
			ZoneName = TEXT("Mountain Zone");
			break;
		case ETerrainZone::Water:
			ZoneName = TEXT("Water Zone");
			break;
		case ETerrainZone::Settlement:
			ZoneName = TEXT("Settlement Zone");
			break;
		}
	}
}
#endif

bool ATerrainZone::IsActorInZone(AActor* Actor) const
{
	if (!Actor)
		return false;

	// Check if actor is within volume bounds
	return EncompassesPoint(Actor->GetActorLocation());
}

TArray<AActor*> ATerrainZone::GetActorsInZone(TSubclassOf<AActor> ActorClass) const
{
	TArray<AActor*> Result;

	// Get all actors overlapping this volume
	TArray<AActor*> OverlappingActors;
	GetOverlappingActors(OverlappingActors, ActorClass);

	return OverlappingActors;
}

FVector ATerrainZone::GetZoneCenter() const
{
	return GetActorLocation();
}

EResourceType ATerrainZone::GetProducedResourceType() const
{
	// Map zone type to resource type
	switch (ZoneType)
	{
	case ETerrainZone::Farmland:
		return EResourceType::Food;
	case ETerrainZone::Pasture:
		return EResourceType::Meat;
	case ETerrainZone::Forest:
		return EResourceType::Wood;
	case ETerrainZone::Mountain:
		// Mountains can produce Stone or Iron (based on richness)
		return (ResourceRichness > 0.7f) ? EResourceType::Iron : EResourceType::Stone;
	case ETerrainZone::Water:
		return EResourceType::Fish;
	case ETerrainZone::Settlement:
		// Settlements don't produce raw resources
		return EResourceType::Food; // Default fallback
	default:
		return EResourceType::Food;
	}
}

int32 ATerrainZone::GatherResources(int32 RequestedAmount)
{
	if (!CanProduceResources())
	{
		return 0;
	}

	// Calculate actual amount based on richness and fertility
	float ProductionMultiplier = ResourceRichness;

	// Fertility affects food production
	if (ZoneType == ETerrainZone::Farmland || ZoneType == ETerrainZone::Pasture)
	{
		ProductionMultiplier *= Fertility;
	}

	// Calculate amount (with some randomness)
	int32 BaseAmount = FMath::RandRange(
		FMath::Max(1, (int32)(RequestedAmount * ProductionMultiplier * 0.5f)),
		(int32)(RequestedAmount * ProductionMultiplier)
	);

	// Slightly deplete the zone over time (very slowly)
	ResourceRichness = FMath::Max(0.1f, ResourceRichness - 0.001f);

	UE_LOG(LogTemp, Log, TEXT("Zone '%s': Gathered %d x %d (Richness: %.3f)"),
		*ZoneName, (int32)GetProducedResourceType(), BaseAmount, ResourceRichness);

	return BaseAmount;
}

bool ATerrainZone::CanProduceResources() const
{
	// Settlement zones don't produce raw resources
	if (ZoneType == ETerrainZone::Settlement)
	{
		return false;
	}

	// Must have some richness remaining
	return ResourceRichness > 0.0f;
}

bool ATerrainZone::AddWorker(ABaseVillager* Worker)
{
	if (!Worker)
	{
		UE_LOG(LogTemp, Warning, TEXT("Zone '%s': Cannot add null worker"), *ZoneName);
		return false;
	}

	// Check if already assigned
	if (AssignedWorkers.Contains(Worker))
	{
		UE_LOG(LogTemp, Warning, TEXT("Zone '%s': Worker %s already assigned"), *ZoneName, *Worker->VillagerName);
		return false;
	}

	// Check capacity
	if (CurrentWorkers >= MaxWorkers)
	{
		UE_LOG(LogTemp, Warning, TEXT("Zone '%s': Full capacity (%d/%d)"), *ZoneName, CurrentWorkers, MaxWorkers);
		return false;
	}

	// Add worker
	AssignedWorkers.Add(Worker);
	CurrentWorkers = AssignedWorkers.Num();

	UE_LOG(LogTemp, Log, TEXT("Zone '%s': Added worker %s (%d/%d)"),
		*ZoneName, *Worker->VillagerName, CurrentWorkers, MaxWorkers);

	return true;
}

bool ATerrainZone::RemoveWorker(ABaseVillager* Worker)
{
	if (!Worker)
	{
		return false;
	}

	// Remove from list
	int32 Removed = AssignedWorkers.Remove(Worker);
	if (Removed > 0)
	{
		CurrentWorkers = AssignedWorkers.Num();
		UE_LOG(LogTemp, Log, TEXT("Zone '%s': Removed worker %s (%d/%d)"),
			*ZoneName, *Worker->VillagerName, CurrentWorkers, MaxWorkers);
		return true;
	}

	return false;
}

bool ATerrainZone::HasAvailableWorkerSlots() const
{
	return CurrentWorkers < MaxWorkers;
}
