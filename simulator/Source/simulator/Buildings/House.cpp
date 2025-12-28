// Copyright Epic Games, Inc. All Rights Reserved.

#include "House.h"
#include "BaseVillager.h"
#include "InventoryComponent.h"

AHouse::AHouse()
{
	// Set building type to House
	BuildingType = EBuildingType::House;
	BuildingName = TEXT("House");

	// Default house capacity
	MaxResidents = 4;
	CurrentResidents = 0;

	// Houses don't need workers
	MaxWorkers = 0;

	// Houses don't need large inventories (just personal storage)
	if (Inventory)
	{
		Inventory->MaxCapacity = 100;
	}
}

void AHouse::BeginPlay()
{
	Super::BeginPlay();

	UE_LOG(LogTemp, Log, TEXT("House '%s' initialized - Capacity: %d/%d"),
		*BuildingName, CurrentResidents, MaxResidents);
}

bool AHouse::HasAvailableSpace() const
{
	return CurrentResidents < MaxResidents;
}

bool AHouse::AddResident(ABaseVillager* Villager)
{
	if (!Villager)
	{
		return false;
	}

	if (!HasAvailableSpace())
	{
		UE_LOG(LogTemp, Warning, TEXT("House '%s' is full, cannot add resident %s"),
			*BuildingName, *Villager->GetName());
		return false;
	}

	if (Residents.Contains(Villager))
	{
		UE_LOG(LogTemp, Warning, TEXT("Villager %s already lives in house '%s'"),
			*Villager->GetName(), *BuildingName);
		return false;
	}

	Residents.Add(Villager);
	CurrentResidents = Residents.Num();

	UE_LOG(LogTemp, Log, TEXT("Villager %s moved into house '%s' (%d/%d)"),
		*Villager->GetName(), *BuildingName, CurrentResidents, MaxResidents);

	return true;
}

bool AHouse::RemoveResident(ABaseVillager* Villager)
{
	if (!Villager)
	{
		return false;
	}

	int32 RemovedCount = Residents.Remove(Villager);

	if (RemovedCount > 0)
	{
		CurrentResidents = Residents.Num();

		UE_LOG(LogTemp, Log, TEXT("Villager %s moved out of house '%s' (%d/%d)"),
			*Villager->GetName(), *BuildingName, CurrentResidents, MaxResidents);

		return true;
	}

	return false;
}

int32 AHouse::GetAvailableSpace() const
{
	return MaxResidents - CurrentResidents;
}
