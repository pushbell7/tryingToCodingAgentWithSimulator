// Copyright Epic Games, Inc. All Rights Reserved.

#include "VillagerManagerSubsystem.h"
#include "BaseVillager.h"
#include "CraftsmanVillager.h"
#include "MerchantVillager.h"
#include "House.h"
#include "TerrainZone.h"
#include "BuildingManagerSubsystem.h"
#include "ZoneManagerSubsystem.h"
#include "EngineUtils.h"
#include "TimerManager.h"

void UVillagerManagerSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	RefreshInterval = 10.0f; // Refresh every 10 seconds
	bAutoAssignOnStart = true;

	// Initial refresh
	RefreshVillagerList();

	// Auto-assign if enabled
	if (bAutoAssignOnStart)
	{
		AutoAssignAll();
	}

	// Set up periodic refresh
	if (RefreshInterval > 0.0f && GetWorld())
	{
		GetWorld()->GetTimerManager().SetTimer(
			RefreshTimerHandle,
			this,
			&UVillagerManagerSubsystem::PeriodicRefresh,
			RefreshInterval,
			true
		);
	}

	UE_LOG(LogTemp, Log, TEXT("VillagerManagerSubsystem initialized - Total population: %d"), AllVillagers.Num());
}

void UVillagerManagerSubsystem::Deinitialize()
{
	// Clear timer
	if (GetWorld())
	{
		GetWorld()->GetTimerManager().ClearTimer(RefreshTimerHandle);
	}

	AllVillagers.Empty();

	Super::Deinitialize();
}

void UVillagerManagerSubsystem::RefreshVillagerList()
{
	AllVillagers.Empty();

	if (!GetWorld())
	{
		return;
	}

	// Find all villagers in the world
	for (TActorIterator<ABaseVillager> It(GetWorld()); It; ++It)
	{
		ABaseVillager* Villager = *It;
		if (Villager)
		{
			AllVillagers.Add(Villager);
		}
	}

	UE_LOG(LogTemp, Log, TEXT("VillagerManagerSubsystem: Refreshed villager list - Found %d villagers"), AllVillagers.Num());
}

void UVillagerManagerSubsystem::AutoAssignAll()
{
	UE_LOG(LogTemp, Log, TEXT("VillagerManagerSubsystem: Starting auto-assignment for %d villagers"), AllVillagers.Num());

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

	UE_LOG(LogTemp, Log, TEXT("VillagerManagerSubsystem: Auto-assignment complete - Homes: %d, Work zones: %d"),
		HomesAssigned, WorkZonesAssigned);
}

bool UVillagerManagerSubsystem::AutoAssignHome(ABaseVillager* Villager)
{
	if (!Villager || !GetWorld())
		return false;

	// Get BuildingManagerSubsystem
	UBuildingManagerSubsystem* BuildingManager = GetWorld()->GetSubsystem<UBuildingManagerSubsystem>();
	if (!BuildingManager)
	{
		UE_LOG(LogTemp, Warning, TEXT("VillagerManagerSubsystem: No BuildingManagerSubsystem found"));
		return false;
	}

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

	UE_LOG(LogTemp, Warning, TEXT("VillagerManagerSubsystem: No available house found for %s"), *Villager->VillagerName);
	return false;
}

bool UVillagerManagerSubsystem::AutoAssignWorkZone(ABaseVillager* Villager)
{
	if (!Villager || !GetWorld())
		return false;

	// Craftsmen are assigned to workshops, not zones
	ACraftsmanVillager* Craftsman = Cast<ACraftsmanVillager>(Villager);
	if (Craftsman)
	{
		// Get BuildingManagerSubsystem
		UBuildingManagerSubsystem* BuildingManager = GetWorld()->GetSubsystem<UBuildingManagerSubsystem>();
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
	// Get ZoneManagerSubsystem
	UZoneManagerSubsystem* ZoneManager = GetWorld()->GetSubsystem<UZoneManagerSubsystem>();
	if (!ZoneManager)
	{
		UE_LOG(LogTemp, Warning, TEXT("VillagerManagerSubsystem: No ZoneManagerSubsystem found"));
		return false;
	}

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

	UE_LOG(LogTemp, Warning, TEXT("VillagerManagerSubsystem: No available work zone found for %s"), *Villager->VillagerName);
	return false;
}

TArray<ABaseVillager*> UVillagerManagerSubsystem::GetVillagersByRole(EVillagerRole VillagerRole) const
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

TArray<ABaseVillager*> UVillagerManagerSubsystem::GetVillagersBySocialClass(ESocialClass VillagerSocialClass) const
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

TArray<ABaseVillager*> UVillagerManagerSubsystem::GetHomelessVillagers() const
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

TArray<ABaseVillager*> UVillagerManagerSubsystem::GetUnemployedVillagers() const
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

int32 UVillagerManagerSubsystem::GetPopulationByRole(EVillagerRole VillagerRole) const
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

int32 UVillagerManagerSubsystem::GetHomelessCount() const
{
	return GetHomelessVillagers().Num();
}

int32 UVillagerManagerSubsystem::GetUnemployedCount() const
{
	return GetUnemployedVillagers().Num();
}

void UVillagerManagerSubsystem::PeriodicRefresh()
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
