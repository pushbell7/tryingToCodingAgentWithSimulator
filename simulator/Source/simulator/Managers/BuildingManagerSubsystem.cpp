// Copyright Epic Games, Inc. All Rights Reserved.

#include "BuildingManagerSubsystem.h"
#include "BaseBuilding.h"
#include "ConstructionSite.h"
#include "ResourceManagerSubsystem.h"
#include "EngineUtils.h"
#include "TimerManager.h"

void UBuildingManagerSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	RefreshInterval = 5.0f; // Refresh every 5 seconds

	// Initial refresh
	RefreshBuildingList();
	RefreshConstructionSites();

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

	UE_LOG(LogTemp, Log, TEXT("BuildingManagerSubsystem initialized with %d buildings, %d construction sites"),
		AllBuildings.Num(), ConstructionSites.Num());
}

void UBuildingManagerSubsystem::Deinitialize()
{
	// Clear timer
	if (GetWorld())
	{
		GetWorld()->GetTimerManager().ClearTimer(RefreshTimerHandle);
	}

	AllBuildings.Empty();
	ConstructionSites.Empty();

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
	RefreshConstructionSites();
}

void UBuildingManagerSubsystem::RegisterBuilding(ABaseBuilding* Building)
{
	if (Building && !AllBuildings.Contains(Building))
	{
		AllBuildings.Add(Building);
		UE_LOG(LogTemp, Log, TEXT("BuildingManagerSubsystem: Registered building %s (Total: %d)"),
			*Building->BuildingName, AllBuildings.Num());
	}
}

void UBuildingManagerSubsystem::UnregisterBuilding(ABaseBuilding* Building)
{
	if (Building)
	{
		AllBuildings.Remove(Building);
		UE_LOG(LogTemp, Log, TEXT("BuildingManagerSubsystem: Unregistered building %s (Total: %d)"),
			*Building->BuildingName, AllBuildings.Num());
	}
}

AConstructionSite* UBuildingManagerSubsystem::CreateConstructionSite(
	FVector Location,
	FRotator Rotation,
	TSubclassOf<ABaseBuilding> BuildingClass,
	EBuildingType BuildingType,
	float RequiredWork,
	int32 MaxWorkers)
{
	if (!GetWorld())
	{
		UE_LOG(LogTemp, Error, TEXT("BuildingManagerSubsystem: Cannot create construction site - no world"));
		return nullptr;
	}

	if (!BuildingClass)
	{
		UE_LOG(LogTemp, Error, TEXT("BuildingManagerSubsystem: Cannot create construction site - no building class"));
		return nullptr;
	}

	// 건물의 기본 객체에서 건설 비용 가져오기
	ABaseBuilding* DefaultBuilding = BuildingClass->GetDefaultObject<ABaseBuilding>();
	if (!DefaultBuilding)
	{
		UE_LOG(LogTemp, Error, TEXT("BuildingManagerSubsystem: Cannot get default building object"));
		return nullptr;
	}

	FConstructionCost Cost = DefaultBuilding->ConstructionCost;

	// 파라미터로 전달된 값이 있으면 오버라이드
	if (RequiredWork > 0.0f)
	{
		Cost.RequiredWorkAmount = RequiredWork;
	}
	if (MaxWorkers > 0)
	{
		Cost.MaxWorkers = MaxWorkers;
	}

	// ResourceManagerSubsystem에서 자원 확인 및 차감
	UResourceManagerSubsystem* ResourceManager = GetWorld()->GetSubsystem<UResourceManagerSubsystem>();
	if (!ResourceManager)
	{
		UE_LOG(LogTemp, Error, TEXT("BuildingManagerSubsystem: No ResourceManagerSubsystem found"));
		return nullptr;
	}

	// 자원이 충분한지 확인
	if (!ResourceManager->CanAffordConstruction(Cost))
	{
		UE_LOG(LogTemp, Warning, TEXT("BuildingManagerSubsystem: Not enough resources for %s construction"),
			*DefaultBuilding->BuildingName);
		ResourceManager->LogResourceStatus();
		return nullptr;
	}

	// 자원 차감
	if (!ResourceManager->PayConstructionCost(Cost))
	{
		UE_LOG(LogTemp, Error, TEXT("BuildingManagerSubsystem: Failed to pay construction cost"));
		return nullptr;
	}

	// 건설 현장 스폰
	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	AConstructionSite* NewSite = GetWorld()->SpawnActor<AConstructionSite>(
		AConstructionSite::StaticClass(),
		Location,
		Rotation,
		SpawnParams
	);

	if (NewSite)
	{
		// 건설 현장 설정
		NewSite->BuildingClass = BuildingClass;
		NewSite->BuildingType = BuildingType;
		NewSite->RequiredWorkAmount = Cost.RequiredWorkAmount;
		NewSite->MaxWorkers = Cost.MaxWorkers;
		NewSite->ConstructionLocation = Location;
		NewSite->BuildingName = FString::Printf(TEXT("%s (Construction)"), *DefaultBuilding->BuildingName);

		// 캐시에 추가
		ConstructionSites.Add(NewSite);

		UE_LOG(LogTemp, Warning, TEXT("BuildingManagerSubsystem: Created construction site for %s at %s (Work: %.0f, Workers: %d)"),
			*NewSite->BuildingName, *Location.ToString(), Cost.RequiredWorkAmount, Cost.MaxWorkers);

		return NewSite;
	}
	else
	{
		// 건설 현장 스폰 실패 시 자원 반환
		UE_LOG(LogTemp, Error, TEXT("BuildingManagerSubsystem: Failed to spawn construction site, refunding resources"));
		ResourceManager->RefundConstructionCost(Cost, Location);
		return nullptr;
	}
}

TArray<AConstructionSite*> UBuildingManagerSubsystem::GetAllConstructionSites() const
{
	return ConstructionSites;
}

AConstructionSite* UBuildingManagerSubsystem::GetNearestConstructionSite(FVector Location) const
{
	AConstructionSite* NearestSite = nullptr;
	float NearestDistance = FLT_MAX;

	for (AConstructionSite* Site : ConstructionSites)
	{
		if (Site && Site->bIsActive && Site->HasAvailableWorkerSlots())
		{
			float Distance = FVector::Dist(Location, Site->GetConstructionLocation());
			if (Distance < NearestDistance)
			{
				NearestDistance = Distance;
				NearestSite = Site;
			}
		}
	}

	return NearestSite;
}

bool UBuildingManagerSubsystem::CancelConstruction(AConstructionSite* Site)
{
	if (!Site)
	{
		return false;
	}

	// 자원 반환
	if (Site->BuildingClass && GetWorld())
	{
		ABaseBuilding* DefaultBuilding = Site->BuildingClass->GetDefaultObject<ABaseBuilding>();
		if (DefaultBuilding)
		{
			UResourceManagerSubsystem* ResourceManager = GetWorld()->GetSubsystem<UResourceManagerSubsystem>();
			if (ResourceManager)
			{
				ResourceManager->RefundConstructionCost(DefaultBuilding->ConstructionCost, Site->GetConstructionLocation());
				UE_LOG(LogTemp, Warning, TEXT("BuildingManagerSubsystem: Refunded construction cost for %s"), *Site->BuildingName);
			}
		}
	}

	ConstructionSites.Remove(Site);
	Site->Destroy();

	UE_LOG(LogTemp, Warning, TEXT("BuildingManagerSubsystem: Cancelled construction of %s"), *Site->BuildingName);
	return true;
}

void UBuildingManagerSubsystem::RefreshConstructionSites()
{
	ConstructionSites.Empty();

	if (!GetWorld())
	{
		return;
	}

	// 월드의 모든 건설 현장 찾기
	for (TActorIterator<AConstructionSite> It(GetWorld()); It; ++It)
	{
		AConstructionSite* Site = *It;
		if (IsValid(Site) && Site->bIsActive)
		{
			ConstructionSites.Add(Site);
		}
	}

	UE_LOG(LogTemp, Log, TEXT("BuildingManagerSubsystem: Found %d active construction sites"), ConstructionSites.Num());
}
