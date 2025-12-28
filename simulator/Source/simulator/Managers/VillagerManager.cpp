// Copyright Epic Games, Inc. All Rights Reserved.

#include "VillagerManager.h"
#include "BaseVillager.h"
#include "CraftsmanVillager.h"
#include "MerchantVillager.h"
#include "House.h"
#include "TerrainZone.h"
#include "BuildingManager.h"
#include "ZoneManager.h"
#include "EngineUtils.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"

AVillagerManager::AVillagerManager()
{
	PrimaryActorTick.bCanEverTick = false;

	RefreshInterval = 10.0f; // Refresh every 10 seconds
	bAutoAssignOnStart = true;
}

void AVillagerManager::BeginPlay()
{
	Super::BeginPlay();

	// Initial refresh
	RefreshVillagerList();

	// Auto-assign if enabled
	if (bAutoAssignOnStart)
	{
		AutoAssignAll();
	}

	// Set up periodic refresh
	if (RefreshInterval > 0.0f)
	{
		GetWorldTimerManager().SetTimer(RefreshTimerHandle, this, &AVillagerManager::PeriodicRefresh, RefreshInterval, true);
	}

	UE_LOG(LogTemp, Log, TEXT("VillagerManager initialized - Total population: %d"), AllVillagers.Num());
}

void AVillagerManager::RefreshVillagerList()
{
	AllVillagers.Empty();

	// Find all villagers in the world
	for (TActorIterator<ABaseVillager> It(GetWorld()); It; ++It)
	{
		ABaseVillager* Villager = *It;
		if (Villager)
		{
			AllVillagers.Add(Villager);
		}
	}

	UE_LOG(LogTemp, Log, TEXT("VillagerManager: Refreshed villager list - Found %d villagers"), AllVillagers.Num());
}

void AVillagerManager::AutoAssignAll()
{
	UE_LOG(LogTemp, Log, TEXT("VillagerManager: Starting auto-assignment for %d villagers"), AllVillagers.Num());

	int32 HomesAssigned = 0;
	int32 WorkZonesAssigned = 0;

	for (ABaseVillager* Villager : AllVillagers)
	{
		if (!Villager)
			continue;

		// Assign home
		if (!Villager->AssignedHome)
		{
			if (AutoAssignHome(Villager))
			{
				HomesAssigned++;
			}
		}

		// Assign work zone
		if (!Villager->AssignedWorkZone)
		{
			if (AutoAssignWorkZone(Villager))
			{
				WorkZonesAssigned++;
			}
		}
	}

	UE_LOG(LogTemp, Log, TEXT("VillagerManager: Auto-assignment complete - Homes: %d, Work zones: %d"),
		HomesAssigned, WorkZonesAssigned);
}

bool AVillagerManager::AutoAssignHome(ABaseVillager* Villager)
{
	if (!Villager)
		return false;

	// Find BuildingManager
	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ABuildingManager::StaticClass(), FoundActors);

	if (FoundActors.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("VillagerManager: No BuildingManager found"));
		return false;
	}

	ABuildingManager* BuildingManager = Cast<ABuildingManager>(FoundActors[0]);
	if (!BuildingManager)
		return false;

	// Get all houses
	TArray<ABaseBuilding*> Buildings = BuildingManager->GetBuildingsByType(EBuildingType::House);

	// Find nearest available house
	AHouse* BestHouse = nullptr;
	float BestDistance = FLT_MAX;

	for (ABaseBuilding* Building : Buildings)
	{
		AHouse* House = Cast<AHouse>(Building);
		if (House && House->HasAvailableSpace())
		{
			float Distance = FVector::Dist(Villager->GetActorLocation(), House->GetBuildingLocation());
			if (Distance < BestDistance)
			{
				BestDistance = Distance;
				BestHouse = House;
			}
		}
	}

	if (BestHouse)
	{
		return Villager->AssignToHome(BestHouse);
	}

	UE_LOG(LogTemp, Warning, TEXT("VillagerManager: No available house found for %s"), *Villager->VillagerName);
	return false;
}

bool AVillagerManager::AutoAssignWorkZone(ABaseVillager* Villager)
{
	if (!Villager)
		return false;

	// Craftsmen are assigned to workshops, not zones
	ACraftsmanVillager* Craftsman = Cast<ACraftsmanVillager>(Villager);
	if (Craftsman)
	{
		// Find BuildingManager
		TArray<AActor*> FoundActors;
		UGameplayStatics::GetAllActorsOfClass(GetWorld(), ABuildingManager::StaticClass(), FoundActors);

		if (FoundActors.Num() == 0)
			return false;

		ABuildingManager* BuildingManager = Cast<ABuildingManager>(FoundActors[0]);
		if (!BuildingManager)
			return false;

		// Get workshops matching specialty
		TArray<ABaseBuilding*> Buildings = BuildingManager->GetBuildingsByType(Craftsman->Specialty);

		// Find nearest available workshop
		ABaseBuilding* BestWorkshop = nullptr;
		float BestDistance = FLT_MAX;

		for (ABaseBuilding* Building : Buildings)
		{
			if (Building && Building->HasAvailableWorkerSlots())
			{
				float Distance = FVector::Dist(Craftsman->GetActorLocation(), Building->GetBuildingLocation());
				if (Distance < BestDistance)
				{
					BestDistance = Distance;
					BestWorkshop = Building;
				}
			}
		}

		if (BestWorkshop)
		{
			return Craftsman->AssignToWorkshop(BestWorkshop);
		}

		return false;
	}

	// Regular villagers get work zones
	// Find ZoneManager
	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AZoneManager::StaticClass(), FoundActors);

	if (FoundActors.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("VillagerManager: No ZoneManager found"));
		return false;
	}

	AZoneManager* ZoneManager = Cast<AZoneManager>(FoundActors[0]);
	if (!ZoneManager)
		return false;

	// Prefer Farmland zones for regular citizens
	TArray<ATerrainZone*> Zones = ZoneManager->GetZonesByType(ETerrainZone::Farmland);

	// Find nearest available zone
	ATerrainZone* BestZone = nullptr;
	float BestDistance = FLT_MAX;

	for (ATerrainZone* Zone : Zones)
	{
		if (Zone && Zone->HasAvailableWorkerSlots())
		{
			float Distance = FVector::Dist(Villager->GetActorLocation(), Zone->GetZoneCenter());
			if (Distance < BestDistance)
			{
				BestDistance = Distance;
				BestZone = Zone;
			}
		}
	}

	// If no farmland available, try other zone types
	if (!BestZone)
	{
		TArray<ETerrainZone> ZoneTypes = { ETerrainZone::Forest, ETerrainZone::Pasture, ETerrainZone::Mountain, ETerrainZone::Water };

		for (ETerrainZone ZoneType : ZoneTypes)
		{
			TArray<ATerrainZone*> AlternateZones = ZoneManager->GetZonesByType(ZoneType);

			for (ATerrainZone* Zone : AlternateZones)
			{
				if (Zone && Zone->HasAvailableWorkerSlots())
				{
					float Distance = FVector::Dist(Villager->GetActorLocation(), Zone->GetZoneCenter());
					if (Distance < BestDistance)
					{
						BestDistance = Distance;
						BestZone = Zone;
					}
				}
			}

			if (BestZone)
				break;
		}
	}

	if (BestZone)
	{
		return Villager->AssignToWorkZone(BestZone);
	}

	UE_LOG(LogTemp, Warning, TEXT("VillagerManager: No available work zone found for %s"), *Villager->VillagerName);
	return false;
}

TArray<ABaseVillager*> AVillagerManager::GetVillagersByRole(EVillagerRole VillagerRole) const
{
	TArray<ABaseVillager*> Result;

	for (ABaseVillager* Villager : AllVillagers)
	{
		if (Villager && Villager->VillagerRole == VillagerRole)
		{
			Result.Add(Villager);
		}
	}

	return Result;
}

TArray<ABaseVillager*> AVillagerManager::GetVillagersBySocialClass(ESocialClass VillagerSocialClass) const
{
	TArray<ABaseVillager*> Result;

	for (ABaseVillager* Villager : AllVillagers)
	{
		if (Villager && Villager->SocialClass == VillagerSocialClass)
		{
			Result.Add(Villager);
		}
	}

	return Result;
}

TArray<ABaseVillager*> AVillagerManager::GetHomelessVillagers() const
{
	TArray<ABaseVillager*> Result;

	for (ABaseVillager* Villager : AllVillagers)
	{
		if (Villager && !Villager->AssignedHome)
		{
			Result.Add(Villager);
		}
	}

	return Result;
}

TArray<ABaseVillager*> AVillagerManager::GetUnemployedVillagers() const
{
	TArray<ABaseVillager*> Result;

	for (ABaseVillager* Villager : AllVillagers)
	{
		if (!Villager)
			continue;

		// Check if regular villager has no work zone
		if (!Villager->AssignedWorkZone)
		{
			// Craftsmen might have workshops instead
			ACraftsmanVillager* Craftsman = Cast<ACraftsmanVillager>(Villager);
			if (!Craftsman || !Craftsman->AssignedWorkshop)
			{
				Result.Add(Villager);
			}
		}
	}

	return Result;
}

int32 AVillagerManager::GetPopulationByRole(EVillagerRole VillagerRole) const
{
	int32 Count = 0;

	for (ABaseVillager* Villager : AllVillagers)
	{
		if (Villager && Villager->VillagerRole == VillagerRole)
		{
			Count++;
		}
	}

	return Count;
}

int32 AVillagerManager::GetHomelessCount() const
{
	return GetHomelessVillagers().Num();
}

int32 AVillagerManager::GetUnemployedCount() const
{
	return GetUnemployedVillagers().Num();
}

void AVillagerManager::PeriodicRefresh()
{
	RefreshVillagerList();

	// Auto-assign any new unassigned villagers
	for (ABaseVillager* Villager : AllVillagers)
	{
		if (!Villager)
			continue;

		if (!Villager->AssignedHome)
		{
			AutoAssignHome(Villager);
		}

		if (!Villager->AssignedWorkZone)
		{
			ACraftsmanVillager* Craftsman = Cast<ACraftsmanVillager>(Villager);
			if (!Craftsman || !Craftsman->AssignedWorkshop)
			{
				AutoAssignWorkZone(Villager);
			}
		}
	}
}
