// Copyright Epic Games, Inc. All Rights Reserved.

#include "ResourceDisplayWidget.h"
#include "Territory.h"

void UResourceDisplayWidget::SetTerritory(ATerritory* InTerritory)
{
	TargetTerritory = InTerritory;
	UpdateResourceDisplay();
}

void UResourceDisplayWidget::UpdateResourceDisplay()
{
	if (!TargetTerritory)
	{
		UE_LOG(LogTemp, Warning, TEXT("ResourceDisplayWidget: No territory set"));
		return;
	}

	// Clear cached data
	CachedResources.Empty();
	TotalResourceAmount = 0;
	MaxStorageCapacity = TargetTerritory->MaxStorageCapacity;

	// Get all resource types
	TArray<EResourceType> AllResourceTypes = {
		// Tier 1
		EResourceType::Food,
		EResourceType::Meat,
		EResourceType::Wood,
		EResourceType::Stone,
		EResourceType::Iron,
		EResourceType::Fish,
		// Tier 2
		EResourceType::Bread,
		EResourceType::Leather,
		EResourceType::Planks,
		EResourceType::Tools,
		EResourceType::Weapons,
		// Tier 3
		EResourceType::Gold,
		EResourceType::Cloth,
		EResourceType::Ale
	};

	// Build display info for each resource
	for (EResourceType ResourceType : AllResourceTypes)
	{
		int32 Amount = TargetTerritory->GetResourceAmount(ResourceType);

		// Only show resources that exist or are important
		if (Amount > 0)
		{
			FResourceDisplayInfo Info;
			Info.ResourceType = ResourceType;
			Info.CurrentAmount = Amount;
			Info.MaxCapacity = MaxStorageCapacity;
			Info.ResourceName = UEnum::GetDisplayValueAsText(ResourceType).ToString();

			CachedResources.Add(Info);
			TotalResourceAmount += Amount;
		}
	}

	// Notify Blueprint to update visual widgets
	OnResourcesUpdated();
}

TArray<FResourceDisplayInfo> UResourceDisplayWidget::GetResourceList() const
{
	return CachedResources;
}

int32 UResourceDisplayWidget::GetResourceAmount(EResourceType ResourceType) const
{
	if (!TargetTerritory)
		return 0;

	return TargetTerritory->GetResourceAmount(ResourceType);
}

float UResourceDisplayWidget::GetStorageUsagePercent() const
{
	if (MaxStorageCapacity <= 0)
		return 0.0f;

	return (float)TotalResourceAmount / (float)MaxStorageCapacity;
}
