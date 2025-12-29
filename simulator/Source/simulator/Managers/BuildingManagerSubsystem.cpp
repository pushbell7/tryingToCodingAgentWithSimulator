// Copyright Epic Games, Inc. All Rights Reserved.

#include "BuildingManagerSubsystem.h"
#include "BaseBuilding.h"
#include "EngineUtils.h"
#include "TimerManager.h"

void UBuildingManagerSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	RefreshInterval = 5.0f; // Refresh every 5 seconds

	// Initial refresh
	RefreshBuildingList();

	// Set up periodic refresh
	if (RefreshInterval > 0.0f && GetWorld())
	{
		GetWorld()->GetTimerManager().SetTimer(
			RefreshTimerHandle,
			this,
			&UBuildingManagerSubsystem::PeriodicRefresh,
			RefreshInterval,
			true
		);
	}

	UE_LOG(LogTemp, Log, TEXT("BuildingManagerSubsystem initialized with %d buildings"), AllBuildings.Num());
}

void UBuildingManagerSubsystem::Deinitialize()
{
	// Clear timer
	if (GetWorld())
	{
		GetWorld()->GetTimerManager().ClearTimer(RefreshTimerHandle);
	}

	AllBuildings.Empty();

	Super::Deinitialize();
}

void UBuildingManagerSubsystem::RefreshBuildingList()
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

	UE_LOG(LogTemp, Log, TEXT("BuildingManagerSubsystem: Found %d buildings"), AllBuildings.Num());
}

TArray<ABaseBuilding*> UBuildingManagerSubsystem::GetBuildingsByType(EBuildingType BuildingType) const
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

ABaseBuilding* UBuildingManagerSubsystem::GetNearestBuilding(FVector Location, EBuildingType BuildingType) const
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

TArray<ABaseBuilding*> UBuildingManagerSubsystem::GetBuildingsWithinRadius(FVector Location, float Radius) const
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

ABaseBuilding* UBuildingManagerSubsystem::GetNearestStorageBuilding(FVector Location) const
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

TArray<ABaseBuilding*> UBuildingManagerSubsystem::GetAllStorageBuildings() const
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

ABaseBuilding* UBuildingManagerSubsystem::GetNearestAvailableStorage(FVector Location) const
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

int32 UBuildingManagerSubsystem::GetBuildingCount() const
{
	return AllBuildings.Num();
}

int32 UBuildingManagerSubsystem::GetBuildingCountByType(EBuildingType BuildingType) const
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

void UBuildingManagerSubsystem::PeriodicRefresh()
{
	RefreshBuildingList();
}
