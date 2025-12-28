// Copyright Epic Games, Inc. All Rights Reserved.

#include "ZoneManager.h"
#include "TerrainZone.h"
#include "Kismet/GameplayStatics.h"
#include "EngineUtils.h"

AZoneManager::AZoneManager()
{
	PrimaryActorTick.bCanEverTick = false;
}

void AZoneManager::BeginPlay()
{
	Super::BeginPlay();

	// Collect all zones in the world
	RefreshZoneList();

	UE_LOG(LogTemp, Log, TEXT("ZoneManager initialized - Found %d zones"), AllZones.Num());
}

void AZoneManager::RefreshZoneList()
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

	UE_LOG(LogTemp, Log, TEXT("ZoneManager: Refreshed zone list - %d zones found"), AllZones.Num());
}

TArray<ATerrainZone*> AZoneManager::GetZonesByType(ETerrainZone ZoneType) const
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

ATerrainZone* AZoneManager::GetNearestZone(FVector Location, ETerrainZone ZoneType) const
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

TArray<ATerrainZone*> AZoneManager::GetZonesWithinRadius(FVector Location, float Radius) const
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

ATerrainZone* AZoneManager::GetZoneAtLocation(FVector Location) const
{
	for (ATerrainZone* Zone : AllZones)
	{
		if (Zone && Zone->IsActorInZone(nullptr))
		{
			// We need to check using the location directly
			if (Zone->EncompassesPoint(Location))
			{
				return Zone;
			}
		}
	}

	return nullptr;
}
