// Copyright Epic Games, Inc. All Rights Reserved.

#include "ZoneManagerSubsystem.h"
#include "TerrainZone.h"
#include "EngineUtils.h"

void UZoneManagerSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// Collect all zones in the world
	RefreshZoneList();

	UE_LOG(LogTemp, Log, TEXT("ZoneManagerSubsystem initialized - Found %d zones"), AllZones.Num());
}

void UZoneManagerSubsystem::Deinitialize()
{
	AllZones.Empty();

	Super::Deinitialize();
}

void UZoneManagerSubsystem::RefreshZoneList()
{
	AllZones.Empty();

	// Find all TerrainZone actors in the world
	for (TActorIterator<ATerrainZone> It(GetWorld()); It; ++It)
	{
		ATerrainZone* Zone = *It;
		if (Zone)
		{
			AllZones.Add(Zone);
		}
	}

	UE_LOG(LogTemp, Log, TEXT("ZoneManagerSubsystem: Refreshed zone list - %d zones found"), AllZones.Num());
}

TArray<ATerrainZone*> UZoneManagerSubsystem::GetZonesByType(ETerrainZone ZoneType) const
{
	TArray<ATerrainZone*> Result;

	for (ATerrainZone* Zone : AllZones)
	{
		if (Zone && Zone->ZoneType == ZoneType)
		{
			Result.Add(Zone);
		}
	}

	return Result;
}

ATerrainZone* UZoneManagerSubsystem::GetNearestZone(FVector Location, ETerrainZone ZoneType) const
{
	ATerrainZone* NearestZone = nullptr;
	float MinDistance = FLT_MAX;

	for (ATerrainZone* Zone : AllZones)
	{
		if (Zone && Zone->ZoneType == ZoneType)
		{
			float Distance = FVector::Dist(Location, Zone->GetZoneCenter());
			if (Distance < MinDistance)
			{
				MinDistance = Distance;
				NearestZone = Zone;
			}
		}
	}

	return NearestZone;
}

TArray<ATerrainZone*> UZoneManagerSubsystem::GetZonesWithinRadius(FVector Location, float Radius) const
{
	TArray<ATerrainZone*> Result;

	for (ATerrainZone* Zone : AllZones)
	{
		if (Zone)
		{
			float Distance = FVector::Dist(Location, Zone->GetZoneCenter());
			if (Distance <= Radius)
			{
				Result.Add(Zone);
			}
		}
	}

	return Result;
}

ATerrainZone* UZoneManagerSubsystem::GetZoneAtLocation(FVector Location) const
{
	for (ATerrainZone* Zone : AllZones)
	{
		if (Zone)
		{
			// Check using the location directly
			if (Zone->EncompassesPoint(Location))
			{
				return Zone;
			}
		}
	}

	return nullptr;
}
