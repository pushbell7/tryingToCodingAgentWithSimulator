// Copyright Epic Games, Inc. All Rights Reserved.

#include "InventoryComponent.h"

UInventoryComponent::UInventoryComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	MaxCapacity = 0; // Unlimited by default
}

int32 UInventoryComponent::AddResource(EResourceType ResourceType, int32 Quantity)
{
	if (Quantity <= 0)
		return 0;

	// Check capacity
	int32 CurrentTotal = GetTotalItems();
	int32 SpaceAvailable = (MaxCapacity > 0) ? (MaxCapacity - CurrentTotal) : Quantity;

	if (SpaceAvailable <= 0)
		return 0;

	// Add what we can
	int32 AmountToAdd = FMath::Min(Quantity, SpaceAvailable);

	if (Resources.Contains(ResourceType))
	{
		Resources[ResourceType] += AmountToAdd;
	}
	else
	{
		Resources.Add(ResourceType, AmountToAdd);
	}

	UE_LOG(LogTemp, Log, TEXT("%s: Added %d x %d (Total: %d)"),
		*GetOwner()->GetName(), (int32)ResourceType, AmountToAdd, Resources[ResourceType]);

	return AmountToAdd;
}

int32 UInventoryComponent::RemoveResource(EResourceType ResourceType, int32 Quantity)
{
	if (Quantity <= 0 || !Resources.Contains(ResourceType))
		return 0;

	int32 CurrentQuantity = Resources[ResourceType];
	int32 AmountToRemove = FMath::Min(Quantity, CurrentQuantity);

	Resources[ResourceType] -= AmountToRemove;

	// Remove entry if quantity reaches 0
	if (Resources[ResourceType] <= 0)
	{
		Resources.Remove(ResourceType);
	}

	UE_LOG(LogTemp, Log, TEXT("%s: Removed %d x %d (Remaining: %d)"),
		*GetOwner()->GetName(), (int32)ResourceType, AmountToRemove,
		Resources.Contains(ResourceType) ? Resources[ResourceType] : 0);

	return AmountToRemove;
}

bool UInventoryComponent::HasResource(EResourceType ResourceType, int32 Quantity) const
{
	if (!Resources.Contains(ResourceType))
		return false;

	return Resources[ResourceType] >= Quantity;
}

int32 UInventoryComponent::GetResourceQuantity(EResourceType ResourceType) const
{
	if (Resources.Contains(ResourceType))
	{
		return Resources[ResourceType];
	}

	return 0;
}

TArray<FResourceStack> UInventoryComponent::GetAllResources() const
{
	TArray<FResourceStack> Result;

	for (const auto& Pair : Resources)
	{
		if (Pair.Value > 0)
		{
			Result.Add(FResourceStack(Pair.Key, Pair.Value));
		}
	}

	return Result;
}

void UInventoryComponent::ClearInventory()
{
	Resources.Empty();
	UE_LOG(LogTemp, Log, TEXT("%s: Inventory cleared"), *GetOwner()->GetName());
}

int32 UInventoryComponent::GetTotalItems() const
{
	int32 Total = 0;

	for (const auto& Pair : Resources)
	{
		Total += Pair.Value;
	}

	return Total;
}

bool UInventoryComponent::IsFull() const
{
	if (MaxCapacity <= 0)
		return false;

	return GetTotalItems() >= MaxCapacity;
}

int32 UInventoryComponent::GetRemainingCapacity() const
{
	if (MaxCapacity <= 0)
		return INT_MAX; // Unlimited

	return MaxCapacity - GetTotalItems();
}
