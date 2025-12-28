// Copyright Epic Games, Inc. All Rights Reserved.

#include "BuildingManager.h"
#include "BaseBuilding.h"
#include "EngineUtils.h"
#include "TimerManager.h"

ABuildingManager::ABuildingManager()
{
	PrimaryActorTick.bCanEverTick = false;
	RefreshInterval = 5.0f; // Refresh every 5 seconds
}

void ABuildingManager::BeginPlay()
{
	Super::BeginPlay();

	// Initial refresh
	RefreshBuildingList();

	// Set up periodic refresh
	if (RefreshInterval > 0.0f)
	{
		GetWorldTimerManager().SetTimer(
			RefreshTimerHandle,
			this,
			&ABuildingManager::PeriodicRefresh,
			RefreshInterval,
			true
		);
	}

	UE_LOG(LogTemp, Log, TEXT("BuildingManager initialized with %d buildings"), AllBuildings.Num());
}

void ABuildingManager::RefreshBuildingList()
{
	AllBuildings.Empty();

	if (!GetWorld())
	{
		return;
	}

	// Find all buildings in the world
	for (TActorIterator<ABaseBuilding> It(GetWorld()); It; ++It)
	{
		ABaseBuilding* Building = *It;
		if (IsValid(Building))
		{
			AllBuildings.Add(Building);
		}
	}

	UE_LOG(LogTemp, Log, TEXT("BuildingManager: Found %d buildings"), AllBuildings.Num());
}

TArray<ABaseBuilding*> ABuildingManager::GetBuildingsByType(EBuildingType BuildingType) const
{
	TArray<ABaseBuilding*> Result;

	for (ABaseBuilding* Building : AllBuildings)
	{
		if (Building && Building->BuildingType == BuildingType)
		{
			Result.Add(Building);
		}
	}

	return Result;
}

ABaseBuilding* ABuildingManager::GetNearestBuilding(FVector Location, EBuildingType BuildingType) const
{
	ABaseBuilding* NearestBuilding = nullptr;
	float NearestDistance = FLT_MAX;

	for (ABaseBuilding* Building : AllBuildings)
	{
		if (Building && Building->BuildingType == BuildingType)
		{
			float Distance = FVector::Dist(Location, Building->GetBuildingLocation());
			if (Distance < NearestDistance)
			{
				NearestDistance = Distance;
				NearestBuilding = Building;
			}
		}
	}

	return NearestBuilding;
}

TArray<ABaseBuilding*> ABuildingManager::GetBuildingsWithinRadius(FVector Location, float Radius) const
{
	TArray<ABaseBuilding*> Result;
	float RadiusSquared = Radius * Radius;

	for (ABaseBuilding* Building : AllBuildings)
	{
		if (Building)
		{
			float DistanceSquared = FVector::DistSquared(Location, Building->GetBuildingLocation());
			if (DistanceSquared <= RadiusSquared)
			{
				Result.Add(Building);
			}
		}
	}

	return Result;
}

ABaseBuilding* ABuildingManager::GetNearestStorageBuilding(FVector Location) const
{
	ABaseBuilding* NearestStorage = nullptr;
	float NearestDistance = FLT_MAX;

	for (ABaseBuilding* Building : AllBuildings)
	{
		if (Building && Building->IsStorageBuilding())
		{
			float Distance = FVector::Dist(Location, Building->GetBuildingLocation());
			if (Distance < NearestDistance)
			{
				NearestDistance = Distance;
				NearestStorage = Building;
			}
		}
	}

	return NearestStorage;
}

TArray<ABaseBuilding*> ABuildingManager::GetAllStorageBuildings() const
{
	TArray<ABaseBuilding*> Result;

	for (ABaseBuilding* Building : AllBuildings)
	{
		if (Building && Building->IsStorageBuilding())
		{
			Result.Add(Building);
		}
	}

	return Result;
}

ABaseBuilding* ABuildingManager::GetNearestAvailableStorage(FVector Location) const
{
	ABaseBuilding* NearestStorage = nullptr;
	float NearestDistance = FLT_MAX;

	for (ABaseBuilding* Building : AllBuildings)
	{
		if (Building && Building->IsStorageBuilding() && Building->CanAcceptResources())
		{
			float Distance = FVector::Dist(Location, Building->GetBuildingLocation());
			if (Distance < NearestDistance)
			{
				NearestDistance = Distance;
				NearestStorage = Building;
			}
		}
	}

	return NearestStorage;
}

int32 ABuildingManager::GetBuildingCount() const
{
	return AllBuildings.Num();
}

int32 ABuildingManager::GetBuildingCountByType(EBuildingType BuildingType) const
{
	int32 Count = 0;

	for (ABaseBuilding* Building : AllBuildings)
	{
		if (Building && Building->BuildingType == BuildingType)
		{
			Count++;
		}
	}

	return Count;
}

void ABuildingManager::PeriodicRefresh()
{
	RefreshBuildingList();
}
