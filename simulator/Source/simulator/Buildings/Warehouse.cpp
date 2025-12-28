// Copyright Epic Games, Inc. All Rights Reserved.

#include "Warehouse.h"
#include "InventoryComponent.h"

AWarehouse::AWarehouse()
{
	// Set building type to Warehouse
	BuildingType = EBuildingType::Warehouse;
	BuildingName = TEXT("Warehouse");

	// Warehouses have large capacity
	if (Inventory)
	{
		Inventory->MaxCapacity = 1000; // Can store 1000 items
	}

	// Warehouses need workers to manage
	MaxWorkers = 2;
}

void AWarehouse::BeginPlay()
{
	Super::BeginPlay();

	UE_LOG(LogTemp, Log, TEXT("Warehouse '%s' initialized - Capacity: %d items"),
		*BuildingName, Inventory ? Inventory->MaxCapacity : 0);
}

float AWarehouse::GetStorageUtilization() const
{
	if (!Inventory || Inventory->MaxCapacity == 0)
	{
		return 0.0f;
	}

	return (float)Inventory->GetTotalItems() / (float)Inventory->MaxCapacity;
}

bool AWarehouse::IsNearlyFull() const
{
	return GetStorageUtilization() >= 0.9f;
}

FString AWarehouse::GetStorageSummary() const
{
	if (!Inventory)
	{
		return TEXT("No inventory");
	}

	TArray<FResourceStack> Resources = Inventory->GetAllResources();

	if (Resources.Num() == 0)
	{
		return TEXT("Empty");
	}

	FString Summary = FString::Printf(TEXT("%d/%d items (%.0f%% full) - "),
		Inventory->GetTotalItems(),
		Inventory->MaxCapacity,
		GetStorageUtilization() * 100.0f);

	// Add resource counts
	for (int32 i = 0; i < Resources.Num(); i++)
	{
		const FResourceStack& Stack = Resources[i];
		Summary += FString::Printf(TEXT("%d x Type%d"),
			Stack.Quantity,
			(int32)Stack.ResourceType);

		if (i < Resources.Num() - 1)
		{
			Summary += TEXT(", ");
		}
	}

	return Summary;
}
