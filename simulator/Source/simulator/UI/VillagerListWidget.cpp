// Copyright Epic Games, Inc. All Rights Reserved.

#include "VillagerListWidget.h"
#include "Territory.h"
#include "BaseVillager.h"
#include "BaseBuilding.h"

void UVillagerListWidget::SetTerritory(ATerritory* InTerritory)
{
	TargetTerritory = InTerritory;
	UpdateVillagerList();
}

void UVillagerListWidget::UpdateVillagerList()
{
	if (!TargetTerritory)
	{
		UE_LOG(LogTemp, Warning, TEXT("VillagerListWidget: No territory set"));
		return;
	}

	// Clear cached data
	CachedVillagers.Empty();
	TotalVillagerCount = 0;
	EmployedVillagerCount = 0;

	// Get all villagers from territory
	for (ABaseVillager* Villager : TargetTerritory->Villagers)
	{
		if (!Villager)
			continue;

		FVillagerDisplayInfo Info;
		Info.VillagerRef = Villager;
		Info.VillagerName = Villager->VillagerName;
		Info.CurrentState = Villager->CurrentState;
		Info.SocialClass = Villager->SocialClass;
		Info.Skills = Villager->Skills;

		// Find assigned building (check all buildings)
		Info.AssignedBuilding = nullptr;
		Info.AssignedBuildingName = TEXT("Unemployed");

		for (ABaseBuilding* Building : TargetTerritory->Buildings)
		{
			if (Building && Building->CurrentWorkers > 0)
			{
				// Check if villager is assigned to this building
				// Note: BaseBuilding doesn't expose AssignedWorkers array
				// We'll need to add a helper function for this
				// For now, just mark as "Employed" if building has workers
			}
		}

		CachedVillagers.Add(Info);
		TotalVillagerCount++;
	}

	// Notify Blueprint
	OnVillagerListUpdated();
}

TArray<FVillagerDisplayInfo> UVillagerListWidget::GetVillagerList() const
{
	return CachedVillagers;
}

FString UVillagerListWidget::GetVillagerSkillText(ABaseVillager* Villager, EBuildingType BuildingType) const
{
	if (!Villager)
		return TEXT("N/A");

	ESkillLevel Level = Villager->GetSkillLevel(BuildingType);
	return UEnum::GetDisplayValueAsText(Level).ToString();
}

ESkillLevel UVillagerListWidget::GetHighestSkillLevel(ABaseVillager* Villager) const
{
	if (!Villager)
		return ESkillLevel::Novice;

	ESkillLevel Highest = ESkillLevel::Novice;

	for (const auto& Pair : Villager->Skills)
	{
		if (Pair.Value > Highest)
		{
			Highest = Pair.Value;
		}
	}

	return Highest;
}

bool UVillagerListWidget::CanVillagerWorkAt(ABaseVillager* Villager, ABaseBuilding* Building) const
{
	if (!Villager || !Building)
		return false;

	return Villager->CanWorkAtBuilding(Building);
}

bool UVillagerListWidget::AssignVillagerToBuilding(ABaseVillager* Villager, ABaseBuilding* Building)
{
	if (!Villager || !Building)
	{
		UE_LOG(LogTemp, Warning, TEXT("VillagerListWidget: Invalid villager or building"));
		return false;
	}

	bool bSuccess = Building->AddWorker(Villager);

	if (bSuccess)
	{
		UE_LOG(LogTemp, Log, TEXT("VillagerListWidget: Assigned %s to %s"),
			*Villager->VillagerName, *Building->BuildingName);
		UpdateVillagerList();
	}

	return bSuccess;
}

bool UVillagerListWidget::UnassignVillager(ABaseVillager* Villager)
{
	if (!Villager || !TargetTerritory)
		return false;

	// Find building this villager is assigned to
	for (ABaseBuilding* Building : TargetTerritory->Buildings)
	{
		if (Building && Building->RemoveWorker(Villager))
		{
			UE_LOG(LogTemp, Log, TEXT("VillagerListWidget: Unassigned %s from %s"),
				*Villager->VillagerName, *Building->BuildingName);
			UpdateVillagerList();
			return true;
		}
	}

	return false;
}
