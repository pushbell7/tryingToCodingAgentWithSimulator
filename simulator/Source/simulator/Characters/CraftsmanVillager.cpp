// Copyright Epic Games, Inc. All Rights Reserved.

#include "CraftsmanVillager.h"
#include "BaseBuilding.h"
#include "InventoryComponent.h"

ACraftsmanVillager::ACraftsmanVillager()
{
	// Craftsmen are commoners by default
	SocialClass = ESocialClass::Commoner;
	VillagerRole = EVillagerRole::Citizen;

	// Default specialty
	Specialty = EBuildingType::Blacksmith;
	AssignedWorkshop = nullptr;
	CraftingEfficiency = 1.0f;

	// Craftsmen carry less inventory (they process, not gather)
	if (Inventory)
	{
		Inventory->MaxCapacity = 30;
	}
}

void ACraftsmanVillager::BeginPlay()
{
	Super::BeginPlay();

	UE_LOG(LogTemp, Log, TEXT("Craftsman '%s' initialized - Specialty: %d, Efficiency: %.2f"),
		*VillagerName, (int32)Specialty, CraftingEfficiency);
}

bool ACraftsmanVillager::AssignToWorkshop(ABaseBuilding* Workshop)
{
	if (!Workshop)
	{
		UE_LOG(LogTemp, Warning, TEXT("Craftsman '%s': Cannot assign to null workshop"), *VillagerName);
		return false;
	}

	if (!Workshop->IsProcessingBuilding())
	{
		UE_LOG(LogTemp, Warning, TEXT("Craftsman '%s': Building '%s' is not a processing building"),
			*VillagerName, *Workshop->BuildingName);
		return false;
	}

	// Unassign from previous workshop if any
	if (AssignedWorkshop)
	{
		UnassignFromWorkshop();
	}

	AssignedWorkshop = Workshop;

	UE_LOG(LogTemp, Log, TEXT("Craftsman '%s' assigned to workshop '%s'"),
		*VillagerName, *Workshop->BuildingName);

	return true;
}

void ACraftsmanVillager::UnassignFromWorkshop()
{
	if (AssignedWorkshop)
	{
		UE_LOG(LogTemp, Log, TEXT("Craftsman '%s' unassigned from workshop '%s'"),
			*VillagerName, *AssignedWorkshop->BuildingName);

		AssignedWorkshop = nullptr;
	}
}

bool ACraftsmanVillager::IsAssigned() const
{
	return AssignedWorkshop != nullptr;
}
