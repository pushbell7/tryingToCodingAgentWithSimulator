// Copyright Epic Games, Inc. All Rights Reserved.

#include "ZoneManagerSubsystem.h"
#include "ZoneGrid.h"
#include "EngineUtils.h"

void UZoneManagerSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// Find zone grid in world
	CachedZoneGrid = nullptr;
	for (TActorIterator<AZoneGrid> It(GetWorld()); It; ++It)
	{
		CachedZoneGrid = *It;
		break; // Assume single zone grid
	}

	if (CachedZoneGrid)
	{
		UE_LOG(LogTemp, Log, TEXT("ZoneManagerSubsystem initialized - Found ZoneGrid with %d cells"),
			CachedZoneGrid->GetTotalCells());
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("ZoneManagerSubsystem initialized - No ZoneGrid found!"));
	}
}

void UZoneManagerSubsystem::Deinitialize()
{
	CachedZoneGrid = nullptr;
	Super::Deinitialize();
}

AZoneGrid* UZoneManagerSubsystem::GetZoneGrid() const
{
	return CachedZoneGrid;
}

ETerrainZone UZoneManagerSubsystem::GetZoneTypeAtLocation(FVector Location) const
{
	if (CachedZoneGrid)
	{
		return CachedZoneGrid->GetZoneTypeAtLocation(Location);
	}

	return ETerrainZone::Farmland; // Default
}
